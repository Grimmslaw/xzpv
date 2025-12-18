#!/usr/bin/env bash

# TODO: add back when you actually understand it
#set -euo pipefail

#------------------------
# Variable Definitions
#------------------------

programname="$(basename "$0")"

LOG_LEVEL=1
COMP_LEVEL=0
DEST=""
DELETE=0
TARGET=""

#------------------------
# Function Definitions
#------------------------

check_getopt() {
    getopt -T >/dev/null 2>&1
    if [ $? -ne 4 ]; then
        echo "To run this script, getopt(1) must be installed and before getopt(3) on the path"
        exit 1
    fi
}

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
  -c <level>, --comp <level>    Level of compression to be used with the xz command.
  -d <dest>, --dest <dest>      Path (or name relative to cwd) of the archive file.
  -l <level>, --log <level>     Threshold for output messages to be printed (0..4, default=1 [ERROR])
  -s, --silent                  Silence output messages. (Equivalent of '-l 0'.)
  -v, --verbose                 Verbose output. (Equivalent of '-l 4'.)
  -x, --delete                  Delete the target folder after archiving.
EOF
}

archive() {
    local -r compression_level=$([ "$COMP_LEVEL" -gt 0 ] && echo "-$COMP_LEVEL" || echo "")
    tar cf - "$TARGET" | pv -s $(($(du -sk "$TARGET" | awk '{print $1}') * 1024)) | xz "$compression_level" > "$ARCHIVE"
    if [ ! -f "$ARCHIVE" ]; then
        log ERROR "Failed to create archive $ARCHIVE"
        exit 1
    fi
}

cleanup() {
    if [[ $DELETE -eq 1 ]]; then
        rm -rf "$TARGET"
    fi
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
        printf "%s\n" "$message"
    fi
}

#------------------------
# Option Parsing
#------------------------

SHORT_OPTS="hc:d:l:svx"
LONG_OPTS="help,comp:,dest:,log:,silent,verbose"
INITIAL_PARSED=$(getopt -o "$SHORT_OPTS" -l "$LONG_OPTS" -n "$programname" -- "$@")
if [ $? -ne 0 ]; then
    log ALL "Error parsing options." >&2
    exit_usage 1
fi

eval set -- "$INITIAL_PARSED"

while true; do
    case "$1" in
        -h|--help)
            exit_usage 0 ;;
        -c|--level)
            if [ -z "$2" ]; then
                log ERROR "Error: Option -$1 requires an argument."
                exit_usage 1
            elif [[ ("$2" -lt 1) || ("$2" -gt 9) ]]; then
                log ERROR "Error: Argument for option -$1 must be between 1 and 9, inclusive."
                exit_usage 1
            fi
            LOG_LEVEL=$(interpret_log_level "$2"); shift; shift ;;
        -d|--dest)
            if [ -z "$2" ]; then
                log ERROR "Error: Option -$1 requires an argument."
                exit_usage 1
            fi
            DEST="$2"; shift; shift ;;
        -l|--log)
            if [ -z "$2" ]; then
                log ERROR "Error: Option -$1 requires an argument."
                exit_usage 1
            fi
            LOG_LEVEL="$2"; shift; shift ;;
        -s|--silent)
            LOG_LEVEL=0; shift ;;
        -v|--verbose)
            LOG_LEVEL=3; shift ;;
        -x|--delete)
            DELETE=1; shift ;;
        *)
            log ERROR "Internal error in option processing ($1)"
            exit_usage 1 ;;
    esac
done

if [[ "$#" -lt 1 ]]; then
    log ERROR "A <target> positional argument is required."
    exit_usage 1
elif [[ "$#" -gt 1 ]]; then
    log VERBOSE "Ignoring extraneous arguments: " "${@:2}"
fi

TARGET="${1}"
ARCHIVE="$DEST".tar.xz

#------------------------
# Program Flow
#------------------------

check_getopt
archive
cleanup
