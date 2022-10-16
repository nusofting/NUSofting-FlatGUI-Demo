PLUGIN_NAME=$1
PLUGIN_VERSION=$2
PLUGIN_BUNDLE_PATH=$3

export ACTION="install"
export SRCROOT="/Users/blurk/Work/framework_plugins/${PLUGIN_NAME}"
export TARGET_TEMP_DIR="/tmp/" #?
export BUILT_PRODUCTS_DIR="$PLUGIN_BUNDLE_PATH"
export INSTALL_ROOT="$TARGET_TEMP_DIR"

export _PRODUCT_PACKAGE_ID="com.NUSofting.${PLUGIN_NAME}"
export _PRODUCT_PACKAGE_VERSION="${PLUGIN_VERSION}"
export _PRODUCT_PACKAGE_NAME=NUSofting_${PLUGIN_NAME}_${PLUGIN_VERSION}_macOS_AU_VST.pkg
export _PRODUCT_PACKAGE_TITLE="${PLUGIN_NAME} ${PLUGIN_VERSION}"
export _PRODUCT_PACKAGE_BUNDLE_DETAILS="(${PLUGIN_NAME}.component,AU,/Library/Audio/Plug-Ins/Components);(${PLUGIN_NAME}.vst,VST,/Library/Audio/Plug-Ins/VST)"

export _COMPONENT_PACKAGE_NAME_BASE="${PLUGIN_NAME}_${PLUGIN_VERSION}"
export _COMPONENT_PACKAGE_NAME_TEMPLATE="${PLUGIN_NAME}_${PLUGIN_VERSION}_@COMPONENT_TYPE.pkg"

export __BUILD_SUPPORT_PATH="/Users/blurk/Work/framework_plugins/Frameworks/Platform/Mac/BuildSupport"
export _TEMPLATE_DISTRIBUTION_FILE="${__BUILD_SUPPORT_PATH}/Template.distribution.xml"
export _TEMPLATE_LICENSE_FILE="${__BUILD_SUPPORT_PATH}/Template.License.rtf"
export _TEMPLATE_WELCOME_FILE="${__BUILD_SUPPORT_PATH}/Template.InstallerWelcome.rtf"

export _BUILT_PACKAGE_COLLECTION_PATH="/Users/blurk/Work/ReleaseArchive"

export _NOTARIZATION_PROFILE="Notarization-blurk"

for SUFFIX in component vst
do
    echo "Codesign $SUFFIX..."
    /usr/bin/codesign \
        --force \
        --sign 75D455AEFAD47AFCB0E5D2294ECE593F2E94DC27 \
        --timestamp \
        --requirements \=designated\ \=\>\ anchor\ apple\ generic\ \ and\ identifier\ \"\$self.identifier\"\ and\ \(\(cert\ leaf\[field.1.2.840.113635.100.6.1.9\]\ exists\)\ or\ \(\ certificate\ 1\[field.1.2.840.113635.100.6.2.6\]\ exists\ and\ certificate\ leaf\[field.1.2.840.113635.100.6.1.13\]\ exists\ \ and\ certificate\ leaf\[subject.OU\]\ \=\ \"LTUSN6SMBB\"\ \)\) \
        --generate-entitlement-der \
        ${PLUGIN_BUNDLE_PATH}/${PLUGIN_NAME}.$SUFFIX
done

echo "🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹🎹"

python3 -u ${__BUILD_SUPPORT_PATH}/CreateInstallerPackage.py
