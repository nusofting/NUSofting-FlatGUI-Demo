#! /usr/bin/env python3
#
# UpdateVersionsAU.py
# (c) 2017 Bernie Maier
#
# Build script for updating Audio Unit version numbers in built products.
# These aren't so easily handled in the standard tool chain because Audio Units
# use packed integer version numbers rather than the typical version strings
# used for bundle versions.
#

import os.path
import plistlib
import sys

class XcodeEnv:
    '''
    Encapsulates the environment provided by Xcode, particularly the relative
    paths to source and various build intermediate and output directories.
    '''
    def __init__(self):
        # The final output directory for the build configuration.
        # This directory is *not* removed by the "clean" build action.
        self.buildOutputDir = self.getXcodeVar('BUILT_PRODUCTS_DIR')

        # The full file path for the prefix header to be built and used when
        # preprocessing the info plist.
        self.plistPrefixHeader = self.getXcodeVar('INFOPLIST_PREFIX_HEADER')

        # Custom variables defined in the xcconfig files in order to feed in
        # names and paths not essential for the main build. We use the naming
        # convention of prefixing with an underscore to highlight these aren't
        # standard Xcode build settings.

        # The version string for the component.
        self.auVersion = self.getXcodeVar('CURRENT_PROJECT_VERSION')

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

def getVersionInt(versionStr):
    '''
    Splits a three-component version string into numbers and constructs an Audio
    Unit version number, which is defined to be the version components packed
    into one byte each. That is, a version string of x.y.z is converted to
    0x00xxyyzz.
    Returns the constructed version number.
    '''
    versionParts = [int(p) for p in versionStr.split('.')]
    numVersionParts = len(versionParts)
    assert numVersionParts == 3, "Version string must have three components, got " + str(numVersionParts) + " from " + versionStr
    versionBytes = 256 * 256 * versionParts[0] + 256 * versionParts[1] + versionParts[2]
    return versionBytes

def createPlistPrefixHeader(xcodeEnv, versionInt):
    '''
    Creates in the projects intermediates directory a prefix header file with a
    simple macro definition defining the version number as an integer, for
    preprocessing into the AU info plist file.
    '''
    prefixHeaderVersionDef = '#define _AU_VERSION ' + str(versionInt) + '\n'
    with open(xcodeEnv.plistPrefixHeader, 'w+') as xc:
        xc.write(prefixHeaderVersionDef)

def main():
    xcodeEnv = XcodeEnv()
    versionInt = getVersionInt(xcodeEnv.auVersion)
    print(xcodeEnv.auVersion, " -> ", versionInt, file=sys.stderr)
    createPlistPrefixHeader(xcodeEnv, versionInt)


if __name__ == "__main__":
    main()
