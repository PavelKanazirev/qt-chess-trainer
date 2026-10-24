#!/usr/bin/env bash
# Day 7 delivery check. Run from the repo root.
pass=0; fail=0
check() {   # check "<description>" <file> <fixed-string>
  if grep -qF -- "$3" "$2" 2>/dev/null; then
    echo "  OK    $1"; pass=$((pass+1))
  else
    echo "  MISS  $1   ($2: '$3')"; fail=$((fail+1))
  fi
}
absent() {  # absent "<description>" <file> <fixed-string>
  if grep -qF -- "$3" "$2" 2>/dev/null; then
    echo "  MISS  $1   ($2 still has '$3')"; fail=$((fail+1))
  else
    echo "  OK    $1"; pass=$((pass+1))
  fi
}

echo "Commit 1: selection in BoardModel"
check "SelectedRole"            src/app/BoardModel.h   "SelectedRole"
check "setSelectedRow"          src/app/BoardModel.h   "setSelectedRow"
check "pieceAt / isOwnPiece"    src/app/BoardModel.h   "isOwnPiece"
check "role name 'selected'"    src/app/BoardModel.cpp '{SelectedRole, "selected"}'
check "setFen clears selection" src/app/BoardModel.cpp "m_selected = -1"
check "selection test"          tests/unit/tst_boardmodel.cpp "selectionFollowsSquareAndClearsOnNewFen"

echo "Commit 2: fenAfterMove in the core"
check "MoveResult field"        src/core/PuzzleSession.h   "fenAfterMove"
check "set in PuzzleSession"    src/core/PuzzleSession.cpp "result.fenAfterMove"
check "MoveOutcome field"       src/core/TrainerTypes.h    "fenAfterMove"
check "copied by facade"        src/core/TrainerService.cpp "out.fenAfterMove"
check "session test"            tests/unit/tst_puzzlesession.cpp "fenAfterMove"

echo "Commit 3: click-to-move in TrainerBridge"
check "feedback property"       src/app/TrainerBridge.h   "Q_PROPERTY(QString feedback"
check "finished property"       src/app/TrainerBridge.h   "Q_PROPERTY(bool finished"
check "availableSelectors"      src/app/TrainerBridge.h   "availableSelectors"
check "squareClicked"           src/app/TrainerBridge.h   "Q_INVOKABLE void squareClicked"
check "setSelector"             src/app/TrainerBridge.h   "Q_INVOKABLE bool setSelector"
check "reply timer"             src/app/TrainerBridge.cpp "QTimer::singleShot"
check "generation counter"      src/app/TrainerBridge.cpp "m_generation"
check "auto-queen"              src/app/TrainerBridge.cpp "isPromotion"
absent "old setStatus removed"  src/app/TrainerBridge.cpp "setStatus("
check "test: selection rules"   tests/unit/tst_trainerbridge.cpp "selectionRules"
check "test: solve by clicking" tests/unit/tst_trainerbridge.cpp "clickingTheSolutionSolvesThePuzzle"
check "test: switch selector"   tests/unit/tst_trainerbridge.cpp "switchesSelector"

echo "Commit 4: interactive board"
check "Square: selected prop"   src/app/qml/Square.qml "required property bool selected"
check "Square: selection tint"  src/app/qml/Square.qml "#7fa650"
check "Square: coordinates"     src/app/qml/Square.qml "charAt(1)"
check "Main: click handler"     src/app/qml/Main.qml   "onSquareClicked"
check "Main: N shortcut"        src/app/qml/Main.qml   "Shortcut"
check "QML test: selected role" tests/qml/tst_board.qml "selected:"
check "QML test: tint test"     tests/qml/tst_board.qml "test_selectedSquareIsTinted"

echo "Commit 5: banner and strategy selector"
check "banner uses feedback"    src/app/qml/SidePanel.qml "trainer.feedback"
check "strategy ComboBox"       src/app/qml/SidePanel.qml "ComboBox"
check "selector list"           src/app/qml/SidePanel.qml "availableSelectors"
check "Next/Skip button"        src/app/qml/SidePanel.qml "trainer.finished"

echo "Commit 6: documentation"
check "Interaction section"     docs/Architecture.md "Interaction"
check "README status"           README.md "Day 7"
if ls docs/img/day07* >/dev/null 2>&1; then echo "  OK    screenshot"; pass=$((pass+1));
else echo "  MISS  screenshot (docs/img/day07-*.png)"; fail=$((fail+1)); fi

echo "Git"
n=$(git rev-list --count main..HEAD)
echo "  INFO  $n commits on this branch (expected about 6)"
dirty=$(git status --porcelain | grep -v 'Commit_map.md' | wc -l)
if [ "$dirty" -eq 0 ]; then echo "  OK    working tree clean"; pass=$((pass+1));
else echo "  MISS  $dirty uncommitted file(s): run git status"; fail=$((fail+1)); fi

echo
echo "Result: $pass OK, $fail missing"
[ "$fail" -eq 0 ]


