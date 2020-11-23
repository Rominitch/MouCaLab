"""
MouCa Asset system
Created by Rominitch for MouCaLab
License: no license
"""
try:
    import os
    import Script
except ImportError as err:
    print("Impossible to import Scripts packages." + str(err) )
    os._exit(-1)

# External program path
zipProgram = os.path.join("C:\Program Files", "7-Zip", "7z.exe")

# Current package description
#             Package                GoogleDrive  ID                      Versionning file
packages = [ ("Inputs-0.01.7z",      "", "IN_0.01"),
             ("References-0.01.7z",  "", "REF_0.01"),
           ]

# Extractor Program
if __name__ == "__main__":
    if os.environ.get('BUILD_MOUCA') is not None:
        utFolder = os.path.join(os.environ['BUILD_MOUCA'])
    else:
        utFolder = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
    utFolder = os.path.join(utFolder, "Bin", "Inputs")

    Script.installAsset(utFolder, packages, zipProgram, "Asset - Setup")

    os._exit(0)
