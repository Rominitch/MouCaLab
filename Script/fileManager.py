"""
MouCa SDK system
Created by Rominitch for MouCaLab
"""
import os
import subprocess
import platform
import shutil

from .console import Log
from .gdd import GoogleDriveDownloader

# Check execution mode
isLocal = os.environ.get('MOUCA_SDK') is not None

def unzip(zipProgram, file, dstFolder):
    # Avoid 7Zip log
    FNULL = open(os.devnull, 'w')
    # Prepare unzip
    arguments = [zipProgram, "x", file, "-o" + dstFolder, "-aos"]

    # Decompress package
    retry = 0
    timeoutMin = 15
    while retry < 3:
        rc = subprocess.run(arguments, timeout=60 * timeoutMin, stdout=FNULL, stderr=subprocess.STDOUT)
        if rc.returncode == 0:
            retry = 3
        else:
            Log.printWarning("Executable running is potentially failed: " + package)
            timeoutMin += 5
            retry += 1
            if retry >= 3:
                Log.printError("Error unzip: " + package + " with error " + str(rc.returncode))

def installSDK(dstFolder, packages, zipProgram, packageName):
    """Download from server and extract 7zip package"""
    def existPackage(dstFolder, package):
        if package[2] is None:
            filename, file_extension = os.path.splitext(package[0])
        else:
            filename = package[2]
        if os.path.exists(os.path.join(dstFolder, filename)):
            Log.printSuccess("Folder " + filename + "\t\talready exist ! Skip package.")
            return True
        else:
            Log.progressMessage("Get package: " + filename + ".")
        return False
    # Go
    installGeneric(dstFolder, packages, zipProgram, packageName, os.environ.get('MOUCA_SDK'), existPackage)

def installAsset(dstFolder, packages, zipProgram, packageName):
    """Download from server and extract 7zip asset"""
    def existPackage(dstFolder, package):
        if package[2] is None:
            raise RuntimeError("Missing versioning asset file")

        if os.path.exists(os.path.join(dstFolder, package[2])):
            Log.printSuccess("Folder " + package[2] + "\t\talready exist ! Skip package.")
            return True
        else:
            Log.progressMessage("Get package: " + package[0] + ".")
        return False
    # Go
    locServer = os.path.join(os.environ.get('MOUCA_SDK'), "Inputs") if os.environ.get('MOUCA_SDK') else None
    installGeneric(dstFolder, packages, zipProgram, packageName, locServer, existPackage)

def installGeneric(dstFolder, packages, zipProgram, packageName, localServer, existPackage):
    """Download from server and extract 7zip generic package"""
    try:
        if not os.path.exists(zipProgram):
            Log.printError("Impossible to find 7Zip at " + zipProgram)
            os._exit(-1)

        Log.blockStop(packageName, "Start install package")

        if platform.system() is not 'Windows':
            raise RuntimeError("Currently Windows OS only is supported.")

        # Load environment variable if needed
        try:
            # Build if not existing
            if not os.path.exists(dstFolder):
                os.makedirs(dstFolder)
            Log.printMessage("Local: " + dstFolder)
        except:
            raise RuntimeError("Impossible to create folder: " + dstFolder)

        error = False

        Log.progressStart("Copy items")
        # Parsing all packages
        for package in packages:
            try:
                # Check already
                if existPackage(dstFolder, package):
                    continue

                # Extract
                if localServer is not None:
                    sourcePackage = localServer
                else:
                    sourcePackage = dstFolder
                    GoogleDriveDownloader.download(package[1], os.path.join(sourcePackage, package[0]), showsize=True,
                                                   overwrite=True)
                # Control
                file = os.path.join(sourcePackage, package[0])
                if not os.path.exists(file):
                    raise RuntimeError("Impossible to get: " + str(file))
                unzip(zipProgram, file, dstFolder)

                # Clean download zip
                if localServer is None:
                    os.remove(file)
            except RuntimeError as e:
                error = True
                Log.printError(str(e))
        Log.progressFinish("End of copy")

        if error:
            raise RuntimeError("Package missing !")

        Log.printSuccess("Package updated: Ready !")
        Log.blockStop(packageName, "End")
    except RuntimeError as e:
        Log.printError(str(e))
        Log.blockStop(packageName, "Finish with errors !")
        os._exit(-1)

def copyDLLs(sdkPath, packages, copyDebug, packageName):
    try:
        Log.blockStart(packageName, "Start copy DLL into binary folder")

        # Prepare Path
        arch = "64"
        unitTestsDir = os.path.realpath(os.path.join(sdkPath, "..", "Bin"))
        unitTestP = os.path.join(unitTestsDir, "Profile"+arch)
        if not os.path.exists(unitTestP):
            os.makedirs(unitTestP)
        unitTestR = os.path.join(unitTestsDir, "Release"+arch)
        if not os.path.exists(unitTestR):
            os.makedirs(unitTestR)
        if copyDebug:
            unitTestD = os.path.join(unitTestsDir, "Debug"+arch)
            if not os.path.exists(unitTestD):
                os.makedirs(unitTestD)

        def copyIfNeeded(file, pathSrc, pathDst):
            fileSrc = os.path.realpath(os.path.join(pathSrc, file))
            if not os.path.exists(fileSrc):
                raise RuntimeError("Impossible to find dll. Path is not correct ?")
            fileDst = os.path.realpath(os.path.join(pathDst, file))
            if os.path.exists(fileDst):
                # in case of the src and dst are the same file
                if os.stat(fileDst).st_mtime - os.stat(fileSrc).st_mtime > 1:
                    Log.printSuccess("Skip up-to-date " + file)
                    return
            shutil.copyfile(fileSrc, fileDst)
            Log.printSuccess("DLL copied: " + file)

        Log.progressStart("Copy items")
        for package, id, path, dll in packages:
            if dll is None:
                continue
            # Create src folder
            filename, file_extension = os.path.splitext(package)
            pathSrc = os.path.join(sdkPath, path if path is not None else filename, dll[0] if dll[0] is not None else "lib64")

            try:
                # Copy release and profile
                copyIfNeeded(dll[1], pathSrc, unitTestR)
                copyIfNeeded(dll[1], pathSrc, unitTestP)
                if copyDebug:
                    copyIfNeeded(dll[2], pathSrc, unitTestD)
            except PermissionError as e:
                Log.printError("DLL copier Failed: No permission " + str(e))
            except:
                Log.printError("DLL copier Failed: " + dll[1])
        Log.progressFinish("End of copy")

        Log.printSuccess("DLL copy: Ready !")
        Log.blockStop(packageName, "End")
    except RuntimeError as e:
        Log.printError(str(e))
        Log.blockStop(packageName, "Finish with errors !")
        os._exit(-1)
