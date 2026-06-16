#!/usr/bin/env bash

MODE=0
FORCE=0

while getopts ":hfu" opt; do
    case $opt in
        h)
            # TODO: help text?
            ;;
        f)
            FORCE=1
            ;;
        u)
            MODE=1
            ;;
        *)
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
    esac
done

# TODO: naive--only tested on my machine

COMMAND="xz-prog"
SCRIPT="$COMMAND.sh"
MANPAGE="$COMMAND.1"
declare -a STATUSES

if [[ $EUID -ne 0 ]]; then
    LOCAL_HOME="$HOME/.local"
    INSTALLED_FOR="USER"
else
    LOCAL_HOME="/usr/local"
    INSTALLED_FOR="ROOT"
fi

OPT_DEST="$LOCAL_HOME/opt"              # e.g., /usr/local/opt
BIN_DEST="$LOCAL_HOME/bin"              # e.g., /usr/local/bin
MAN_DEST="$LOCAL_HOME/share/man/man1"   # e.g., /usr/local/share/man/man1
VAR_DEST="$LOCAL_HOME/var/$COMMAND"     # e.g., /usr/local/var/xz-prog

OPT_BIN="$OPT_DEST/$COMMAND/bin"        # e.g., /usr/local/opt/xz-prog/bin
OPT_SCRIPT="$OPT_BIN/$SCRIPT"           # e.g., /usr/local/opt/xz-prog/xz-prog.sh
BIN_SCRIPT="$BIN_DEST/$COMMAND"         # e.g., /usr/local/bin/xz-prog

OPT_MAN="$OPT_DEST/$COMMAND/man"        # e.g., /usr/local/opt/man
OPT_MANPAGE="$OPT_MAN/$MANPAGE"         # e.g., /usr/local/opt/man/xz-prog.1
MAN_MANPAGE="$MAN_DEST/$MANPAGE"        # e.g., /usr/local/share/man/man1/xz-prog.1

mandb_exists() {
    if command -v mandb >/dev/null 2>&1; then
        return 0
    else
        return 1
    fi
}

script_action() {
    case $MODE in
        0)
            mkdir -p "$OPT_BIN"
            cp "$SCRIPT" "$OPT_SCRIPT"
            chmod +x "$OPT_SCRIPT"
            ln -s "$OPT_SCRIPT" "$BIN_SCRIPT"
            ;;
        1)
            if [[ $FORCE -eq 0 ]]; then
                rm -i "$BIN_SCRIPT"
                rm -ri "$OPT_BIN"
            else
                rm -f "$BIN_SCRIPT"
                rm -rf "$OPT_BIN"
            fi
            ;;
    esac
}

manpage_action() {
    case $MODE in
        0)
            mkdir -p "$OPT_MAN"
            cp "$MANPAGE" "$OPT_MANPAGE"
            ln -s "$OPT_MANPAGE" "$MAN_MANPAGE"
            if mandb_exists; then
                mandb
            else
                STATUSES+=("mandb command not detected. Installed manpage may not be immediately visible.")
            fi
            ;;
        1)
            if [[ $FORCE -eq 0 ]]; then
                rm -i "$MANPAGE"
                rm -ri "$OPT_MAN"
            else
                rm -f "$MANPAGE"
                rm -rf "$OPT_MAN"
            fi
            ;;
    esac
}

environment_action() {
    case $MODE in
        0)
            echo "scope = $INSTALLED_FOR" >> "$VAR_DEST/config.toml"
            ;;
        1)
            if [[ $FORCE -eq 0 ]]; then
                rm -i "$VAR_DEST/config.toml"
            else
                rm -f "$VAR_DEST/config.toml"
            fi
            ;;
    esac
}

script_action
manpage_action
environment_action
for status in "${STATUSES[@]}"; do
    echo "$status"
done
