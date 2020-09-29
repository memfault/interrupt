#!/bin/bash
set -euo pipefail

function help()
{
  echo "\
Usage: interrupt-cli [--build] [--run]
  Build, and run interrupt blog

Options:
  -b, --build           Build and install required packages.
  -r, --run             Launch blog at http://localhost:4000.
  -h, --help            Display this help and exit.
"
}

OPTSTR=brh
LONGOPTS=build,run,help

BUILD=false
RUN=false

! PARAMS=$(getopt --options=$OPTSTR --longoptions=$LONGOPTS --name "$0"  -- "$@")

if [ "${PIPESTATUS[0]}" -ne 0 ]; then
    exit 2
fi

eval set -- "$PARAMS"

while true; do
  case "$1" in
    -h|--help)
      shift
      help
      exit 0
      ;;
    -b|--build)
      BUILD=true
      shift 1
      ;;
    -r|--run)
      RUN=true
      shift 1
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "Error found while parsing arguments!"
      exit 3
      ;;
  esac
done


if [ $# -ne 0 ]; then
    echo "$0: Option \"$1\" not supported"
    exit 4
fi

LAUNCH_INTERRUPT=""
CONCATENATION_IS_REQUIRED=false

concatenate_if_required()
{
  if [ $CONCATENATION_IS_REQUIRED = true ]; then
      LAUNCH_INTERRUPT+=" && "
  fi
}

if [ $BUILD = true ]; then
  concatenate_if_required
  LAUNCH_INTERRUPT+="bundle install"
  CONCATENATION_IS_REQUIRED=true
fi


if [ $RUN = true ]; then
  concatenate_if_required
  LAUNCH_INTERRUPT+="RUBYOPT='-W0' bundle exec jekyll serve -D"
fi

echo "$LAUNCH_INTERRUPT"
eval "$LAUNCH_INTERRUPT"
