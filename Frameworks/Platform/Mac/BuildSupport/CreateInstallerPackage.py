#! /usr/bin/env python3
#
# CreateInstallerPackage.py
# (c) 2013 Bernie Maier
#
# Build script for creating a standard OS X installer package for the built
# plugin.
#

import errno
import os
import os.path
import subprocess
import shutil
import sys

class XcodeEnv:
    '''
    Encapsulates the environment provided by Xcode, particularly the relative
    paths to source and various build intermediate and output directories.
    '''
    def __init__(self):
        action = self.getXcodeVar('ACTION')
        if action != "install":
            raise EnvironmentError('This script must be run via an Xcode "archive" build action')

        # The directory containing the Xcode project (.xcodeproj) directory.
        # The source files can be found relative to this directory.
        self.srcPath = os.path.relpath(self.getXcodeVar('SRCROOT'))

        # The target-specific directory containing intermediate built files.
        # This is one of the directories removed by the "clean" build action.
        self.derivedFilePath = self.getXcodeVar('TARGET_TEMP_DIR')

        # The final output directory for the build configuration.
        # This directory is *not* removed by the "clean" build action.
        self.buildOutputDir = self.getXcodeVar('BUILT_PRODUCTS_DIR')

        # The final output directory for the archive build action.
        # This directory is removed by Xcode when the archive build action completes.
        self.archiveOutputDir = self.getXcodeVar('INSTALL_ROOT')

        # Custom variables defined in the xcconfig files in order to feed in
        # names and paths not essential for the main build. We use the naming
        # convention of prefixing with an underscore to highlight these aren't
        # standard Xcode build settings.

        # The identifier to be given to the package to create.
        self.packageId = self.getXcodeVar('_PRODUCT_PACKAGE_ID')

        # The file system name of the package to create. This should be the
        # product name potentially decorated by configuration-dependent info
        # plus the version, but essentially it's up to the xcconfig to define
        # the rules.
        self.packageName = self.getXcodeVar('_PRODUCT_PACKAGE_NAME')

        # The human-readable title for the packages to create. This is
        # constructed in the xcconfig in a similar manner to the name, but
        # better suited for readability.
        self.packageTitle = self.getXcodeVar('_PRODUCT_PACKAGE_TITLE')

        # The version number of the product being built.
        self.version = self.getXcodeVar('_PRODUCT_PACKAGE_VERSION')

        # The details of the bundles to package. These are formatted as a string
        # of semi colon-separated tuples, where each tuple is of the form
        # (name,component type,path) and name is the file system name of the
        # bundle (expected to be built in BUILD_PRODUCTS_DIR) and path is the
        # path the bundle is to be installed into.
        bundleDetails = self.getXcodeVar('_PRODUCT_PACKAGE_BUNDLE_DETAILS').split(';') # build a list of tuples
        # Now iterate over the list of tuples, stripping the parenthesis, splitting
        # the tuple at the comma, and converting them into a dictionary.
        keys = [ 'sourcePath', 'componentType', 'installPath' ]
        self.bundleDetails = [dict(zip(keys, bd.strip('()').split(','))) for bd in bundleDetails]

        # Common base part of the component package name.
        self.componentPkgNameBase = self.getXcodeVar('_COMPONENT_PACKAGE_NAME_BASE')

        # A template for constructing the name of the component packages.
        self.componentPkgNameTemplate = self.getXcodeVar('_COMPONENT_PACKAGE_NAME_TEMPLATE')

        # The path to the license file to include in the installer.
        self.licenseFile = self.getXcodeVar('_TEMPLATE_LICENSE_FILE')

        # The path to the template welcome file, which will be expanded with the product name
        # and included in the installer.
        self.welcomeFile = self.getXcodeVar('_TEMPLATE_WELCOME_FILE')

        # The path to the template distribution file, which will be expanded with the product name,
        # version and base bundle ID, and included in the installer.
        self.distributionFile = self.getXcodeVar('_TEMPLATE_DISTRIBUTION_FILE')

        # Helper path for collecting built packages.
        # This makes it easier to find the built installer packages, which Xcode builds
        # deep in the derived data directory tree.
        self.packageCollectionDir = self.getXcodeVar('_BUILT_PACKAGE_COLLECTION_PATH')

        # Notarization profile.
        # Requires an app-specific password to be created for `notarytool` via
        # the AppleID associated with the Apple Developer Program account.
        # Then, that app-specific password is associated with the AppleID and
        # developer program team and stored in the macOS keychain on the build
        # machine via:
        #    `xcrun notarytool store-credentials --apple-id <aid> --team-id <team>`
        # This command prompts for a profile name and the password. Then, the
        # notarization credentials can just be fetched from the keychain by
        # `notarytool` using the name.
        # A good write-up for this (possibly easier than Apple's docs) is:
        # https://scriptingosx.com/2021/07/notarize-a-command-line-tool-with-notarytool/
        self.notarizationProfile = self.getXcodeVar('_NOTARIZATION_PROFILE')

    @staticmethod
    def getXcodeVar(name):
        '''
        Returns a non-empty string value for the named environment variable
        (assumed to be one set by Xcode).  If the environment variable is not
        defined or an empty string, raises an EnvironmentError exception.
        '''
        value = os.getenv(name)
        if not value:
            raise EnvironmentError('This script must be run from within Xcode')
        return value


def buildComponentPackage(xcodeEnv, pkgId, name, bundle):
    '''
    Builds the component package for a single bundle.

    @param pkgId
        The package ID to use for the component package.
    @param name
        The file system name for the built package.
    @param bundle
        Details of the bundle to be packaged.
    '''

    print('buildComponentPackage:', file=sys.stderr)
    print('    pkgId:', pkgId, file=sys.stderr)
    print('    name:', name, file=sys.stderr)
    print('    bundle:', bundle, file=sys.stderr)
    # The build output directory for Xcode archive builds actually contains symbolic links to the built products, but
    # the package building command needs a real path. So build the path name and convert to a real path.
    componentPath = os.path.realpath(os.path.join(xcodeEnv.buildOutputDir, bundle['sourcePath']))
    print('    component:', componentPath, file=sys.stderr)
    #pkgbuild --component ${BUILD_DIR}/Noisetar.component --identifier com.nusofting.noisetar-au.pkg --version ${VERSION} --install-location /Library/Audio/Plug-Ins/Components /tmp/Noisetar-${VERSION}-AU.pkg
    pkgbuildArgs = [ 'pkgbuild',
                     '--component',        componentPath,
                     '--identifier',       pkgId,
                     '--version',          str(xcodeEnv.version),
                     '--install-location', bundle['installPath'],
                     os.path.join(xcodeEnv.derivedFilePath, name) ]
    runCommand(pkgbuildArgs)

def buildProduct(xcodeEnv, dist, name, pkgDir, resDir):
    '''
    Builds the product package from the previously built component packages,
    distribution file and resources..

    @param dist
        The distribution file describing the overall installer UI and specifying
        the component packages.
    @param name
        The file system name for the built package.
    @param pkgDir
        The intermediate directory containing the component packages.
    @param resDir
        The source directory containing additional installer resources.
    '''

    #productbuild --distribution ${DIST} --version ${VERSION} --resources ${INSTALLER_SUPPORT_FILES_BASE}/Resources --package-path /tmp ${BUILD_DIR}/${PKG_NAME}
    productbuildArgs = [ 'productbuild',
                         '--distribution', dist,
                         '--version',      str(xcodeEnv.version),
                         '--resources',    resDir,
                         '--package-path', pkgDir,
                         '--sign',         "Developer ID",
                         os.path.join(xcodeEnv.archiveOutputDir, name) ]
    runCommand(productbuildArgs)
    # Copy the package from the archive output directory (deep in the derived data tree) to an easily found directory.
    finalDest = os.path.join(xcodeEnv.packageCollectionDir, name)
    shutil.copy(os.path.join(xcodeEnv.archiveOutputDir, name), finalDest)
    print("Copied installer package to", finalDest, file=sys.stderr)

    print('-' * 80, file=sys.stderr)
    print('Notarizing. Please wait...', file=sys.stderr)

    notarizeArgs = [ 'xcrun',
                     'notarytool',
                     'submit', finalDest,
                     '--keychain-profile', xcodeEnv.notarizationProfile,
                     '--wait' ]
    runCommand(notarizeArgs)

    print('-' * 80, file=sys.stderr)
    print('Stapling...', file=sys.stderr)

    stapleArgs = [ 'xcrun',
                   'stapler',
                   'staple', '-v', finalDest ]
    runCommand(stapleArgs)


def runCommand(commandAndArgs):
    '''
    Runs the specified command, prints the output and raises an OSError if the
    command did not return 0.

    @param commandAndArgs
        The list containing the command to run and its arguments.
    '''

    print(commandAndArgs[0], 'args:', commandAndArgs, file=sys.stderr)
    #return
    p = subprocess.Popen(commandAndArgs,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    output, err = p.communicate()
    print(output.decode('utf-8'), file=sys.stderr)
    print(err.decode('utf-8'), file=sys.stderr)
    if p.returncode != 0:
        raise OSError(commandAndArgs[0] + ' command failed')

def generateInstallerFileFromTemplate(xcodeEnv, templatePath, destinationDir):
    '''
    Creates, from a template file, a file used by the installer to display
    message text in an installer pane. The template file can contain $VARIABLE
    variables which are substituted here.

    @param templatePath
        The full path to the template file used as the text source.
    @param destinationDir
        The directory to contain the generated text file.
    @return
        Returns the file name (including path) of the generated text file. This
        name is derived from the template file name, removing the "Template."
        part of the name.
    '''
    fileTemplate = os.path.normpath(templatePath)
    fileName = os.path.basename(fileTemplate).replace('Template.', '')
    filePath = os.path.join(destinationDir, fileName)
    print('file template -> name:', fileTemplate, fileName, file=sys.stderr)
    with open(fileTemplate, 'r') as infile:
        template = infile.read()
        generatedText = template.replace('$PRODUCT_TITLE', xcodeEnv.packageTitle)
        generatedText = generatedText.replace('$VERSION', str(xcodeEnv.version))
        generatedText = generatedText.replace('$BUNDLE_ID_BASE', xcodeEnv.packageId)
        generatedText = generatedText.replace('$PACKAGE_NAME', xcodeEnv.componentPkgNameBase)
        with open(filePath, 'w') as outfile:
            outfile.write(generatedText)
    return filePath


def main():
    xcodeEnv = XcodeEnv()

    for bundle in xcodeEnv.bundleDetails:
        print('-' * 80, file=sys.stderr)
        print('Adding bundle:', bundle, file=sys.stderr)
        pkgId = xcodeEnv.packageId + '.' + bundle['componentType'] + '.pkg'
        pkgName = xcodeEnv.componentPkgNameTemplate.replace('@COMPONENT_TYPE', bundle['componentType'])
        buildComponentPackage(xcodeEnv, pkgId, pkgName, bundle)

    print('-' * 80, file=sys.stderr)
    destResDir = os.path.join(xcodeEnv.derivedFilePath, 'Resources')
    try:
        os.mkdir(destResDir)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(destResDir):
            pass
        else:
            raise
    generateInstallerFileFromTemplate(xcodeEnv, os.path.normpath(xcodeEnv.licenseFile), destResDir)
    generateInstallerFileFromTemplate(xcodeEnv, os.path.normpath(xcodeEnv.welcomeFile), destResDir)
    distributionFileName = generateInstallerFileFromTemplate(xcodeEnv, os.path.normpath(xcodeEnv.distributionFile), xcodeEnv.derivedFilePath)

    print('-' * 80, file=sys.stderr)
    buildProduct(xcodeEnv, distributionFileName, xcodeEnv.packageName, xcodeEnv.derivedFilePath, destResDir)
    print('-' * 80, file=sys.stderr)

if __name__ == "__main__":
    main()
