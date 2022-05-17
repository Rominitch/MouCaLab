"""
MouCa SDK system
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
#             Package                    GoogleDrive  ID                     Path on disk                            DLL to copy (path, release, debug)
packages = [ ("assimp-5.0.0.rc1.7z",    "13T0WsgNH8Kl8eEiGeYMzv2yawFCJQZli", None,                                   (None, "assimp5.0.0rc1.dll", "assimp5.0.0rc1D.dll")),
             ("Boost-1.79.0.7z",        "1MveZGBgP8VkUaQz3OrAVkdRWvB1yM9Xm", None,                                   (None)),
             ("FreeImage-3.180.7z",     "1x8QCnw4X6xegu8OwLF6CqjB-H5G2_xT0", None,                                    (os.path.join("Dist", "x64"), "FreeImage.dll", "FreeImaged.dll")),
             ("Freetype-2.11.0C.7z",    "1MG2aou6ofB1UN4MdENjYamC4BLRq0iHf", None,                                    None),
             ("glfw-3.3.4.7z",          "1x2J-iTEZ8Q8EI8_0U7wsji47bEKdYukG", None,                                    None),
             ("glm-0.9.9.8.7z",         "1Ec6aQyTo7DGd79w52OhjL67jgWKlEq4g", None,                                    None),
             ("googletest-1.11.0.7z",   "1ihKXNSp5KWUKFb9F25PK8evjQLThlRuc", None,                                    None),
             ("imgui-1.87.7z",          "11I0sPOdxBNmdXi6hAryUk9Z7x2O5QXNl", None,                                    None),
             ("KTX-Software-4.0.0.7z",  "1hEhjq4b_ICktO7cq2f1YzPx_eNOommnF", None,                                    (os.path.join("bin"), "ktx.dll", "ktx.dll")),
             ("openvr-1.16.8.7z",       "1s9EjMLeH-5zxkG4ozqB4Mong_KD1LiWK", None,                                    (os.path.join("bin", "win64"), "openvr_api.dll", "openvr_api.dll")),
             ("sqlite-3.38.7z",         "1qKtm1TsEoG189Nlw-kSfDH2oSWQjb82m", None,                                    None),
             ("SteamWorksSDK-146.7z",   "1fmSY_sBH7SKa9r1eZDInCejV_nxbrFEN", None,                                    (os.path.join("redistributable_bin", "win64"), "steam_api64.dll", "steam_api64.dll")),
             ("Vulkan-1.3.211.0.7z",    "1tKLH_nHVWehN1hJCrhZ6PFG2ngndPTOL", os.path.join("VulkanSDK", "1.3.211.0"),  None),
             ("xerces-c-3.2.2.7z",      "1heXsPWp28zHR8NFNThqaio8MMhLeoC-J", None,                                   (None, "xerces-c_3_2.dll", "xerces-c_3_2D.dll")),
             ("include-0.04.7z",        "1KY-TVun0kew8RXNjSGhPFuQKEt4bnLTB", os.path.join("include", "INCLUDE-0.04"), None),
           ]

# Extractor Program
if __name__ == "__main__":
    if os.environ.get('BUILD_MOUCA') is not None:
        sdkFolder = os.path.join(os.environ['BUILD_MOUCA'], "SDK")
    else:
        sdkFolder = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../SDK/")

    Script.installSDK(sdkFolder, packages, zipProgram, "SDK - Setup")

    Script.copyDLLs(sdkFolder, packages, True, "SDK - Copy")

    os._exit(0)