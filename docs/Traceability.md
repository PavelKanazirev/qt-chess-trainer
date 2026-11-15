# Requirements traceability

Each requirement from [ProblemDefinition.md](ProblemDefinition.md), the code that implements
it, the tests that verify it, and where the result differs from the first plan.

| Req | Implementation | Verified by | Notes |
|---|---|---|---|
| R1 load puzzles from CSV | `PuzzleParser`, `PuzzleRepository` | `tst_puzzleparser`, `tst_puzzlerepository`, `tst_lichessdata` | 200 Lichess puzzles (CC0), 40 per rating band |
| R2 QML board, click-click moves | `BoardModel`, `Board.qml`, `Square.qml`, `TrainerBridge::squareClicked` | `tst_boardmodel`, `tst_qml`, `tst_trainerbridge` | promotion is always to a queen |
| R3 validate moves, auto-play replies | `ChessPosition` (adapter over chess-library), `MoveValidator`, `PuzzleSession` | `tst_chessposition`, `tst_movevalidator`, `tst_puzzlesession`, `tst_trainerbridge` | any other mating move is also accepted |
| R4 selection strategy switchable at runtime | `IPuzzleSelector` and three strategies, `SelectorRegistry`, `TrainerService::setSelector` | `tst_selectors`, `tst_selectorregistry`, `tst_trainerservice`, `tst_trainerconcurrency` | |
| R5 statistics | `StatsStore` | `tst_statsstore`, `tst_trainerflow` | **changed:** one overall Elo rating, plus solved/failed per theme |
| R6 client REST API | `ClientApiServer`, `ApiJson` | `tst_apijson`, `tst_clientapi`, `smoke` | **changed:** `POST /sessions` and `POST /sessions/{id}/moves`, see [API.md](API.md) |
| R7 admin REST API with token | `AdminApiServer`, `ApiServerBase` | `tst_adminapi`, `smoke`, `admin_e2e` | token check before routing, constant-time comparison |
| R8 JSON configuration | `ConfigService`, `config/config.default.json` | `tst_configservice`, `tst_adminapi` | ports and data path apply after a restart |

## Acceptance

| Criterion | Verified by |
|---|---|
| ctest passes | CI job `build-test` (22 tests) |
| the app starts and shows a puzzle | `smoke`, manual check |
| the admin GUI changes the strategy and the app reacts | `tools/admin_gui/tests`, `admin_e2e`, `tst_trainerbridge::followsChangesMadeOutsideTheBridge`, manual check |

## Design patterns

| Pattern | Where |
|---|---|
| Strategy | `IPuzzleSelector`: Random, ByRating, WeakestTheme |
| Facade | `TrainerService`: the only entry point for QML and both APIs |
| Observer | `ConfigService` → `TrainerService` → `TrainerBridge` |
| Adapter | `ChessPosition` (chess-library), `TrainerBridge` (facade to QML) |
| Template Method | `ApiServerBase::start` calls the subclass's `registerRoutes` |
| Simple Factory | `SelectorRegistry` creates strategies by name |
