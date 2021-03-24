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
             ("Boost-1.72.0.7z",        "1pl9Umsn_Durfc42msqnPCmWo6hHot2En", None,                                   (None)),
             ("Boost-1.72.0-stage.7z",  "1slDcGzU8qyTbnNapK5OJi7Mo69ye8EUV", os.path.join("Boost-1.72.0", "lib64"),   None),
             ("FreeImage-3.180.7z",     "1x8QCnw4X6xegu8OwLF6CqjB-H5G2_xT0", None,                                    (os.path.join("Dist", "x64"), "FreeImage.dll", "FreeImaged.dll")),
             ("Freetype-2.10.2.7z",     "1zF5L61Fo4m3alCdq1xHk_l0C_tJDsMYL", None,                                    None),
             ("glfw-3.3.2.7z",          "1AhjRGokc5o3BavC8kNUmS7Vd9q7pU18_", None,                                    None),
             ("gli-0.8.2.0.7z",         "1eqmp5Er6H0RHfUtWUsi87-qQOdWhCHE4", None,                                    None),
             ("glm-0.9.8.5.7z",         "1ThZEnM5KlpSfts_OOp8slHiKLpLjy5hN", None,                                    None),
             ("googletest-1.10.0.7z",   "1I8tvgpFE03YUChGELMtFTpqfmuzo0g-Q", None,                                    (None, "gtest.dll", "gtestd.dll")),
             ("imgui-1.79.7z",          "1TXhxPLP7cfOn1oji8zFDY5wJTH6JM52v", None,                                    None),
             ("KTX-Software-2.0.2.7z",  "14BMn8iaKB2k2t0_5yAnECPxruXCQjWyl", None,                                    None),
             ("openvr-1.11.11.7z",      "1rptiPRPQNDEJ8Et_K8q3T4kTvkv8A92O", None,                                    (os.path.join("bin", "win64"), "openvr_api.dll", "openvr_api.dll")),
             ("sqlite-3.33.7z",         "15Zv4Iy_l1mCext05ZuvPHaPXv7X37LJb", None,                                    None),
             ("SteamWorksSDK-146.7z",   "1fmSY_sBH7SKa9r1eZDInCejV_nxbrFEN", None,                                    (os.path.join("redistributable_bin", "win64"), "steam_api64.dll", "steam_api64.dll")),
             ("Vulkan-1.2.162.1.7z",    "1ChbuCNmqv0YxNlDcS_lTS4oL-Jklkm-u", os.path.join("VulkanSDK", "1.2.162.1"),  None),
             ("xerces-c-3.2.2.7z",      "1heXsPWp28zHR8NFNThqaio8MMhLeoC-J", None,                                   (None, "xerces-c_3_2.dll", "xerces-c_3_2D.dll")),
             ("include-0.01.7z",        "1NjNgJaoMr30gxn62C5nETMZFt9Z3_sdN", os.path.join("include", "INCLUDE-0.01"), None),
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