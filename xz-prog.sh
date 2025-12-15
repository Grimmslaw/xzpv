#!/usr/bin/env bash

# TODO: add back when you actually understand it
#set -euo pipefail

getopt -T >/dev/null 2>&1

if [ $? -ne 4 ]; then
    echo "To run this script, getopt(1) must be installed and before getopt(3) on the path"
    exit 1
fi

programname="$(basename "$0")"

LOG_LEVEL=1
DEST=""
DELETE=0
FORCE_DELETE=0
TARGET=""

exit_usage() {
    local status=1
    if [[ $# -gt 0 ]] && [[ "$1" = "0" ]]; then
        status=0
    fi

    if [[ $LOG_LEVEL -gt 0 ]]; then
        usage
    fi

    exit "$status"
}

usage() {
    cat <<EOF
Usage: $programname [options] <target>

Options:
  -d <dest>, --dest <dest>  Path (or name relative to cwd) of the archive file.
  -v, --verbose             Verbose output.
  -x, --delete              Delete the target folder after archiving.
  -f, --delete-force        Delete the target folder after archiving (using the -f flag for the 'rm' command)
  -l <level>, --log <level> Threshold for messages to be printed (0..4, default=1 [ERROR])
EOF
}

interpret_log_level() {
    local -r level=$(echo "$1" | tr '[:lower:]' '[:upper:]')
    case "$level" in
        SILENT|0)
            echo "0" ;;
        ERROR|1)
            echo "1" ;;
        INFO|2)
            echo "2" ;;
        VERBOSE|3)
            echo "3" ;;
        DEBUG|4)
            echo "4" ;;
        ALL)
            echo "ALL" ;;
        *)
            echo "1" ;;
    esac
}

log() {
    if [[ "$#" -lt 2 ]]; then
        return 1
    fi

    local -r log_at=$(interpret_log_level "$1")
    local -r message="$2"
    if [[ ( ( "$LOG_LEVEL" -ne 0 ) && ( ( "$log_at" = "ALL" ) || ( "$log_at" -ge "$LOG_LEVEL" ) ) ) ]]; then
        printf "%s\n" $message
    fi
}

INITIAL_PARSED=$(getopt -o "hd:xfl:v" -l "help,dest:,delete,delete-force,log:,verbose,debug" -n "$programname" -- "$@")
if [ $? -ne 0 ]; then
    log ALL "Error parsing options." >&2
    exit_usage 1
fi
eval set -- "$INITIAL_PARSED"
while true; do
    case "$1" in
        --debug)
            LOG_LEVEL=4; shift ;;
        -h|--help)
            exit_usage 0 ;;
        -l|--log)
            if [ -z "$2" ]; then
                log ERROR "Error: Option -$1 requires an argument." >&2
                exit_usage 1
            fi
            LOG_LEVEL=$(interpret_log_level "$2"); shift; shift ;;
        -d|--dest)
            if [ -z "$2" ]; then
                log ERROR "Error: Option -$1 requires an argument." >&2
                exit_usage 1
            fi
            DEST="$2"; shift; shift  ;;
        -v|--verbose)
            LOG_LEVEL=3; shift ;;
        -f|--delete-force)
            DELETE=1; FORCE_DELETE=1; shift ;;
        -x|--delete)
            DELETE=1; shift ;;
        --)
            shift; break ;;
        *)
            log ERROR "Internal error in option processing ($1)" >&2
            exit_usage 1 ;;
    esac
done

log DEBUG "programname=$programname, DEST=$DEST, VERBOSE=$VERBOSE, DELETE=$DELETE, @=$*"

if [[ "$#" -lt 1 ]]; then
    log ERROR "A <target> positional argument is required."
    exit_usage 1
elif [[ "$#" -gt 1 ]]; then
    log VERBOSE "Ignoring extraneous arguments: " "${@:2}"
fi

TARGET="${1}"
ARCHIVE="$DEST".tar.xz

tar cf - "$TARGET" | pv -s $(($(du -sk "$TARGET" | awk '{print $1}') * 1024)) | xz > "$ARCHIVE"
if [ ! -f "$ARCHIVE" ]; then
    log ERROR "Failed to create archive $ARCHIVE"
    exit 1
fi

if [[ $FORCE_DELETE -eq 1 ]]; then
    rm -rf "$TARGET"
elif [[ $DELETE -eq 1 ]]; then
    rm -r "$TARGET"
fi

