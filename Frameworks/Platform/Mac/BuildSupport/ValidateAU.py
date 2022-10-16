#! /usr/bin/env python3
#
# ValidateAU.py
# (c) 2017 Bernie Maier
#
# Build script for running Audio Unit validation.
#

import argparse
import os
import os.path
import subprocess
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

        # The actual architectures built.
        self.architectures = self.getXcodeVar('ARCHS')

        # Custom variables defined in the xcconfig files in order to feed in
        # names and paths not essential for the main build. We use the naming
        # convention of prefixing with an underscore to highlight these aren't
        # standard Xcode build settings.

        # Computed path for the built component
        self.auBuildDir = self.getXcodeVar('_BUILT_SOURCE_PATH')

        # The directory the AU is installed into for a local install.
        self.auLocalInstallDir = self.getXcodeVar('_LOCAL_INSTALL_PATH')

        # The manufacturer four character code.
        self.manufacturer4cc = self.getXcodeVar('_AU_MANUFACTURER_4CC')

        # The four character code identifying the plugin (a.k.a the subtype).
        self.pluginId4cc = self.getXcodeVar('_AU_IDENTIFIER_4CC')

        # The Audio Unit type four character code.
        self.auType4cc = self.getXcodeVar('_AU_TYPE_4CC')

        # The directory the AU is built into (varies depending on whether or not this is an archive build).
        action = self.getXcodeVar('ACTION')
        self.isReleaseBuild = action == "install"

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


def validateAudioUnit(xcodeEnv, architecture):
    '''
    Runs the auvaltool Audio Unit validation tool for the plugin specified in
    the Xcode environment and the specified architecture.
    '''

    auvalArgs = [ 'auval',
                  '-v',
                  xcodeEnv.auType4cc,
                  xcodeEnv.pluginId4cc,
                  xcodeEnv.manufacturer4cc ]
    runCommand(auvalArgs, architecture)

def runCommand(commandAndArgs, architecture):
    '''
    Runs the specified command for the specified architecture, prints the output
    and raises an OSError if the command did not return 0.

    @param commandAndArgs
        The list containing the command to run and its arguments.
    @param architecture
        The architecture to run under.
    '''

    archCommandAndArgs = [ 'arch', '-' + architecture ]
    archCommandAndArgs.extend(commandAndArgs)
    print('    ', archCommandAndArgs[0], 'args:', archCommandAndArgs, file=sys.stderr)
    #return
    p = subprocess.Popen(archCommandAndArgs, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = p.communicate()
    outputLines = output.decode('utf-8').split('\n')
    print('\n'.join(outputLines[5:13]), file=sys.stderr)
    print('...', file=sys.stderr)
    print(outputLines[-3], file=sys.stderr)
    if p.returncode != 0:
        print(err.decode('utf-8'), file=sys.stderr)
        raise OSError(commandAndArgs[0] + ' command failed for architecture ' + architecture)

def rsyncCommand(source, destination, exactMatch):
    rsyncArgs = [ 'rsync',
                  '-aiv',
                  source,
                  destination ]
    if exactMatch:
        rsyncArgs.append('--delete-after')
    print('    ', rsyncArgs[0], 'args:', rsyncArgs, file=sys.stderr)
    p = subprocess.Popen(rsyncArgs, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = p.communicate()
    print(output.decode('utf-8'), file=sys.stderr)
    if err:
        print("Errors:", file=sys.stderr)
        print(err.decode('utf-8'), file=sys.stderr)
    if p.returncode != 0:
        raise OSError(rsyncArgs[0] + ' command failed')

def buildArgParser():
    parser = argparse.ArgumentParser(description='Install the built Audio Unit locally and validate it')
    parser.add_argument('--skip-validation', '-s', action='store_true', help='skip the AU validation step, i.e. only install the AU')
    return parser

def main():
    parser = buildArgParser()
    parsedArgs = parser.parse_args()
    xcodeEnv = XcodeEnv()

    print('Locally installing built Audio Unit:', file=sys.stderr)
    rsyncCommand(xcodeEnv.auBuildDir, xcodeEnv.auLocalInstallDir, xcodeEnv.isReleaseBuild)
    if not parsedArgs.skip_validation:
        for architecture in xcodeEnv.architectures.split(' '):
            print('-' * 80, file=sys.stderr)
            validateAudioUnit(xcodeEnv, architecture)

if __name__ == "__main__":
    main()
