SCRIPT_FILE="$1"
shift
PYTHON="/usr/bin/env python3"
PYTHON_VERSION=$($PYTHON --version)

echo "================================================================================"
echo "Running custom build phase script: $SCRIPT_FILE $* via $PYTHON_VERSION" 
$PYTHON -u "${_BUILD_SCRIPT_PATH}/${SCRIPT_FILE}" $*
RES=$?
echo "${SCRIPT_FILE} returned ${RES}"
echo "================================================================================"
exit "${RES}"
