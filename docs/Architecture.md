# Architecture

## Admin: change the selection strategy

Source: diagrams/admin-config-change.puml (CI renders it to SVG and uploads it as the
"diagrams" artifact of every workflow run).

The admin GUI sends PUT /config with a token. The admin server verifies the token,
ConfigService validates and persists the change, and TrainerService swaps.
