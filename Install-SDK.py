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
             ("Boost-1.76.0.7z",        "19LK6miLn-JPI3q9bjun1ZvBLLxOfqMfr", None,                                   (None)),
             ("FreeImage-3.180.7z",     "1x8QCnw4X6xegu8OwLF6CqjB-H5G2_xT0", None,                                    (os.path.join("Dist", "x64"), "FreeImage.dll", "FreeImaged.dll")),
             ("Freetype-2.10.2.7z",     "1zF5L61Fo4m3alCdq1xHk_l0C_tJDsMYL", None,                                    None),
             ("glfw-3.3.4.7z",          "1x2J-iTEZ8Q8EI8_0U7wsji47bEKdYukG", None,                                    None),
             ("glm-0.9.9.8.7z",         "1Ec6aQyTo7DGd79w52OhjL67jgWKlEq4g", None,                                    None),
             ("googletest-1.10.0.7z",   "1I8tvgpFE03YUChGELMtFTpqfmuzo0g-Q", None,                                    (None, "gtest.dll", "gtestd.dll")),
             ("imgui-1.83.7z",          "1-3cZlv-UfKiEFv0kOeXAm0K_zTY9WYNZ", None,                                    None),
             ("KTX-Software-4.0.0.7z",  "1hEhjq4b_ICktO7cq2f1YzPx_eNOommnF", None,                                    (os.path.join("bin"), "ktx.dll", "ktx.dll")),
             ("openvr-1.16.8.7z",       "1s9EjMLeH-5zxkG4ozqB4Mong_KD1LiWK", None,                                    (os.path.join("bin", "win64"), "openvr_api.dll", "openvr_api.dll")),
             ("sqlite-3.36.7z",         "1JAH1m3JkPvTEVz8Ok0CCjaYiuydUD5CD", None,                                    None),
             ("SteamWorksSDK-146.7z",   "1fmSY_sBH7SKa9r1eZDInCejV_nxbrFEN", None,                                    (os.path.join("redistributable_bin", "win64"), "steam_api64.dll", "steam_api64.dll")),
             ("Vulkan-1.2.176.1.7z",    "17dzvRmx5Rb9vXgma8SwPTc5D_X6PU9jg", os.path.join("VulkanSDK", "1.2.176.1"),  None),
             ("xerces-c-3.2.2.7z",      "1heXsPWp28zHR8NFNThqaio8MMhLeoC-J", None,                                   (None, "xerces-c_3_2.dll", "xerces-c_3_2D.dll")),
             ("include-0.02.7z",        "12zQJq8rtz65VtBPaqPWpUcx7uuNv_wT7", os.path.join("include", "INCLUDE-0.02"), None),
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