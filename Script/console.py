"""
MouCa SDK system
Created by Rominitch for MouCaLab
"""
import os
import sys

# Get TeamCity python report if available
if os.environ.get('IS_TEAMCITY') is not None:
    local = os.path.dirname(os.path.abspath(__file__))
    sys.path.append(os.path.join(local, "..", "..", "Dashboard"))
    import teamCity as Log
else:
    try:
        import colorama
        from termcolor import colored

        colorama.init()
        colorConsole = True
    except:
        colorConsole = False

    class Log:
        """Void"""
        @staticmethod
        def progressStart(string):
            print(string)
        def progressFinish(string):
            print(string)
        def progressMessage(string):
            print(string)
        def printSuccess(string):
            if colorConsole:
                print(colored(string, 'green'))
            else:
                print(string)
        def printWarning(string):
            if colorConsole:
                print(colored(string, 'yellow'))
            else:
                print(string)
        def printError(string):
            if colorConsole:
                print( colored(string, 'red') )
            else:
                print(string)
        def printMessage(string):
            print(string)
        def blockStart(blockName, blockDescription):
            print(blockDescription)
        def blockStop(blockName, blockDescription):
            print(blockDescription)
        def compilationStart(blockName):
            pass
        def compilationStop(blockName):
            pass
        def testStart(executableName):
            print(executableName)
        def testStop(executableName):
            pass
        def testFailed(executableName, message):
            Log.printError(executableName+" "+message)