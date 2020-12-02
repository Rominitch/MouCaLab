/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "include/MouCaLab.h"

#include <LibRT/include/RTBufferCPU.h>
#include <LibRT/include/RTCamera.h>
#include <LibRT/include/RTCameraComportement.h>
#include <LibRT/include/RTCameraManipulator.h>
#include <LibRT/include/RTFrustum.h>
#include <LibRT/include/RTLight.h>
#include <LibRT/include/RTMesh.h>
#include <LibRT/include/RTObject.h>

#include <LibGUI/include/GUIOutline.h>
#include <LibGUI/include/GUIBezier.h>
#include <LibGUI/include/GUIFontSVGManager.h>

#include <LibVulkan/include/VKContextDevice.h>
#include <LibVulkan/include/VKCommand.h>
#include <LibVulkan/include/VKCommandBuffer.h>
#include <LibVulkan/include/VKDescriptorSet.h>
#include <LibVulkan/include/VKSequence.h>
#include <LibVulkan/include/VKSubmitInfo.h>
#include <LibVulkan/include/VKWindowSurface.h>

#include "include/MouCaLab.h"

struct TimeElapsed
{
    double _elapsed;
    std::chrono::steady_clock::time_point _start;

    TimeElapsed():
    _elapsed(0.0)
    {}

    void reset()
    {
        _elapsed = 0.0;
    }

    void start()
    {
        _start = std::chrono::high_resolution_clock::now();
    }

    void cumulate()
    {
        const auto end = std::chrono::high_resolution_clock::now();
        _elapsed += std::chrono::duration<double, std::milli>(end - _start).count();
    }
};

#define MAX_VISIBLE_GLYPHS 4096
#define NUMBER_OF_GLYPHS 96

struct Specialization
{
    int32_t   _sampling;
};

struct GlyphInstance
{
    GUI::BoundingBox2D     rect;
    alignas(4) uint32_t  glyph_index;
    alignas(4) float       sharpness;

    GlyphInstance():
    glyph_index(0), sharpness(1.0f)
    {}
};

class FontSVGTest : public MouCaLabTest
{
    public:
        inline RT::BufferDescriptor getGlyphInstanceDescriptor()
        {
            const std::vector<RT::ComponentDescriptor> descriptors =
            {
                { 4, RT::Type::Float,  RT::ComponentUsage::Vertex  },
                { 1, RT::Type::UInt32, RT::ComponentUsage::Anonyme },
                { 1, RT::Type::Float,  RT::ComponentUsage::Anonyme },
            };
            RT::BufferDescriptor bufferDescriptor;
            bufferDescriptor.initialize(descriptors);
            return bufferDescriptor;
        }

        struct UBOScreen
        {
            glm::mat4 projectionMatrix;
            glm::mat4 viewMatrix;
        };

        struct Models
        {
            RT::CameraSPtr             _camera;

            RT::BufferLinkedCPUSPtr    _uboTexts;

            UBOScreen                  _screenParameters;
            RT::BufferLinkedCPUSPtr    _uboScreenVS;
            RT::BufferCPUSPtr          _specialization;

            RT::BufferCPUSPtr          _ubo2GlyphInfo;
            RT::BufferCPUSPtr          _ubo2GlyphControls;
            RT::BufferCPUSPtr          _ubo2GlyphPoints;
        };

        // CPU Asset
        RT::Array2ui    _resolution;
        Models          _models;
        RT::ObjectSPtr  _supportCamera;
        RT::BoundingBox _sceneBBox;

        glm::vec2            _canvasOffset;
        float                _canvasScale;
        GUI::FontSVGManager  _fontSVGManager;

        GUI::FontFamilySVGWPtr _fontFamilyRoboto;
        GUI::FontFamilySVGWPtr _fontFamilyNoto;
        GUI::FontFamilySVGWPtr _fontFamilyCeltic;
        GUI::FontFamilySVGWPtr _fontFamilyMevNo;
        
        std::array<GlyphInstance, MAX_VISIBLE_GLYPHS> _glyphInstances;
        uint32_t                                      _glyphCount;

        // Performances / GUI
        std::array<bool, 4> _modeGUI = { false, false, false, false };
        uint32_t      _nbChars;
        TimeElapsed   _charAppends;
        TimeElapsed   _fontBuild;

        uint32_t      _startDemo;

        static std::vector<const char*> _textDebug;
        static std::vector<const char*> _text;
        static std::vector<const char*> _textASCII;
        static std::vector<const char*> _textU8;

        static Core::Path _fontRoboto;
        static Core::Path _fontNoto;
        static Core::Path _fontNotoJP;
        static Core::Path _fontNotoA;
        static Core::Path _fontCeltic;
        static Core::Path _fontMevNo;

        struct Text
        {
            const std::vector<const char*>& _text;
            glm::vec2                 _offset;
            float                     _scale;
            float                     _interline;
            GUI::FontFamilySVGWPtr*   _font;

            Text(const std::vector<const char*>& text, const glm::vec2& offset, GUI::FontFamilySVGWPtr* font, const float scale = 1.0f, const float interline = 30.0f) :
            _text(text), _offset(offset), _font(font), _scale(scale), _interline(interline)
            {}
        };

        FontSVGTest():
        _resolution(1280, 720), _nbChars(0), _glyphCount(0),
        _startDemo(0),
        _canvasOffset(0.0f, 0.0f), _canvasScale(0.7f)
        {
            _modeGUI[_startDemo] = true;
        }

        char32_t toUtf32(const std::string& s)
        {
            try
            {
                std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
                auto asInt = convert.from_bytes(s);
                return std::u32string(reinterpret_cast<char32_t const*>(asInt.data()), asInt.length()).front();
            }
            catch (...)
            {
                return U' ';
            }
        }

        bool append_text(GUI::FontFamilySVG& font, glm::vec2 offset, float scale, const char* text)
        {
            bool update = false;
            _charAppends.start();

            while (*text)
            {
                ++_nbChars;

                if (_glyphCount >= MAX_VISIBLE_GLYPHS)
                    break;

                int byteGlyph = 1;
                if (~(*text) & 0x80)
                {
                }
                else if ((*text & 0xF0) == 0xF0)
                {
                    byteGlyph = 4;
                }
                else if ((*text & 0xE0) == 0xE0)
                {
                    byteGlyph = 3;
                }
                else if ((*text & 0xC0) == 0xC0)
                {
                    byteGlyph = 2;
                }

                const auto glyph = toUtf32(std::string(text, byteGlyph));

                GUI::FontFamilySVG::GlyphSVG gi;
                update |= font.tryGlyph(glyph, gi);
                GlyphInstance& inst = _glyphInstances[_glyphCount];

                const auto& bbox = gi._outline.getBoundingBox();
//                 inst.rect._min.x = x + bbox._min.x * scale;
//                 inst.rect._min.y = y - bbox._min.y * scale;
//                 inst.rect._max.x = x + bbox._max.x * scale;
//                 inst.rect._max.y = y - bbox._max.y * scale;

                inst.rect._min = offset + glm::vec2(bbox._min.x, -bbox._min.y) * scale;
                inst.rect._max = offset + glm::vec2(bbox._max.x, -bbox._max.y) * scale;

                {
                    inst.glyph_index = gi._glyphID;
                    inst.sharpness   = scale;

                    _glyphCount++;

                    const std::vector<RT::Point3> points
                    {
                        RT::Point3(inst.rect._min, 0.0f), RT::Point3(inst.rect._max, 0.0f)
                    };
                    _sceneBBox.expand(points);
                }

                std::advance(text, byteGlyph);
                offset.x += gi._advance * scale;
            }

            _charAppends.cumulate();
            return update;
        }

        void configureEventManager() override
        {
            _eventManager = std::make_shared<EventManager3DText>();

            // Build camera
            _supportCamera = std::make_shared<RT::Object>();
            _supportCamera->getOrientation().setPosition(glm::vec3(0.0f, 0.0f, 0.0f));

            auto trackball = std::make_shared<RT::CameraManipulator>();
            trackball->initialize(_models._camera);
            trackball->installComportement(RT::CameraManipulator::TrackBallCamera);
            static_cast<RT::CameraTrackBall*>(trackball->getComportement())->setDepthRange(0.001f, 400.0f);
            static_cast<RT::CameraTrackBall*>(trackball->getComportement())->moveTo({ 0.0f, 0.0f, 1.0f });
            
            // Attach support
            static_cast<RT::CameraTrackBall*>(trackball->getComportement())->attachSupport(_supportCamera);

            auto flying = std::make_shared<RT::CameraManipulator>();
            flying->initialize(_models._camera);
            flying->installComportement(RT::CameraManipulator::FlyingCamera);

            // Register into event Manager
            auto eventManager = dynamic_cast<EventManager3DText*>(_eventManager.get());
            eventManager->setSpeedWheel(0.0001f);
            ASSERT_NO_THROW(eventManager->addManipulator(trackball));
            ASSERT_NO_THROW(eventManager->addManipulator(flying));
        }

        //bool buildText(GUI::FontFamilySVGWPtr font, std::vector<const char*> text, const glm::vec2& offset, const float scale, const float interLine)
        bool buildText(const Text& text)
        {
            MOUCA_PRE_CONDITION(!text._font->expired());

            bool update = false;

            for (auto i = 0; i < text._text.size(); i++)
            {
                update |= append_text(*text._font->lock(),
                    text._offset + glm::vec2(0.0f, i * text._interline),
                    text._scale,
                    text._text[i]);
            }
            return update;
        }
        
        void buildFontFamilies(GUI::FontSVGManager& fontSVGManager)
        {
            auto& resources = _core.getResourceManager();
            const auto& fontPath = resources.getResourceFolder(MouCaCore::ResourceManager::Fonts);

            _fontBuild.start();
            fontSVGManager.initialize();

            _fontFamilyRoboto = fontSVGManager.createFont({ fontPath / _fontRoboto });
            _fontFamilyNoto   = fontSVGManager.createFont({ fontPath / _fontNoto, fontPath / _fontNotoJP, fontPath / _fontNotoA });
            _fontFamilyCeltic = fontSVGManager.createFont({ fontPath / _fontCeltic });
            _fontFamilyMevNo  = fontSVGManager.createFont({ fontPath / _fontMevNo  });

            _fontBuild.cumulate();
        }

        void createGlyphBuffer(GUI::FontSVGManager& fontSVGManager)
        {
            _fontBuild.start();

            // Transfer Glyphs to custom buffer
            _models._uboTexts->create(RT::BufferDescriptor(sizeof(GlyphInstance)), _glyphInstances.size(), _glyphInstances.data());

            fontSVGManager.buildFontBuffersV2(*_models._ubo2GlyphInfo, *_models._ubo2GlyphPoints.get(), *_models._ubo2GlyphControls.get());

            _fontBuild.cumulate();
        }

        void releaseFont(GUI::FontSVGManager& fontSVGManager)
        {
            // Remove local link
            _fontFamilyRoboto.reset();
            _fontFamilyNoto.reset();

            // Release memory
            _fontBuild.start();
            fontSVGManager.release();
            _fontBuild.cumulate();
        }
        
        void demoPage(const uint32_t idPage, GUI::FontSVGManager& fontSVGManager)
        {
            const std::array<std::vector<Text>, 4> _pages
            {{
                //Page 0 
                { Text(_text, glm::vec2(-0.7f, -0.51f), &_fontFamilyRoboto, 0.0003f, 0.045f) },
                //{ Text(_text, glm::vec2(0.0f, 0.0f), &_fontFamilyRoboto, 1.0f, 0.045f) },
                //Page 1 
                {
                    Text(_textASCII, glm::vec2(-0.7f, -0.51f), &_fontFamilyRoboto, 0.0003f, 0.045f),
                    Text(_textASCII, glm::vec2(-0.7f, -0.35f), &_fontFamilyMevNo,  0.0003f, 0.045f),
                    Text(_textASCII, glm::vec2(-0.7f, -0.10f), &_fontFamilyNoto,   0.0003f, 0.045f),
                    Text(_textASCII, glm::vec2(-0.7f,  0.10f), &_fontFamilyCeltic, 0.0003f, 0.045f),
                },
                //Page 2
                { Text(_textU8, glm::vec2(-0.7f, -0.3f), &_fontFamilyNoto, 0.0012f, 0.16f) },
                //Page 3
                { Text(_textDebug, glm::vec2(-0.85f, 0.2f), &_fontFamilyNoto, 0.0027f, 0.45f) },
                //{ Text(_textDebug, glm::vec2(0.0f, 0.0f), &_fontFamilyNoto, 10.0f, 0.45f) },
            }};

            // Build page from empty manager
            {
                // Build all fonts from scratch
                buildFontFamilies(fontSVGManager);
                _glyphCount = 0;
                _nbChars    = 0;
                bool update = false;
                _sceneBBox = RT::BoundingBox();

                // Build all texts
                for(const auto& text : _pages[idPage])
                {
                    update |= buildText(text);
                }
                // Update glyph buffer if needed
                if (update)
                {
                    _models._ubo2GlyphInfo->release();
                    _models._ubo2GlyphPoints->release();
                    _models._ubo2GlyphControls->release();
                    _models._uboTexts->release();
                    createGlyphBuffer(fontSVGManager);
                }
            }
            releaseFont(fontSVGManager);
        }

        void makeScene()
        {
            _models._camera = std::make_shared<RT::Camera>();

            // Configure camera
            configureEventManager();

            // Make linked buffer (no memory management)
            _models._uboScreenVS = std::make_shared<RT::BufferLinkedCPU>();
            _models._uboScreenVS->create(RT::BufferDescriptor(sizeof(UBOScreen)), 1, &_models._screenParameters);

            // Specialization for shader
            {
                _models._specialization = std::make_shared<RT::BufferCPU>();
                Specialization* buffer = reinterpret_cast<Specialization*>(_models._specialization->create(RT::BufferDescriptor(sizeof(Specialization)), 1));
                buffer->_sampling = VK_SAMPLE_COUNT_8_BIT;
            }

            _models._uboTexts       = std::make_shared<RT::BufferLinkedCPU>();

            _models._ubo2GlyphInfo     = std::make_shared<RT::BufferCPU>();
            _models._ubo2GlyphPoints   = std::make_shared<RT::BufferCPU>();
            _models._ubo2GlyphControls = std::make_shared<RT::BufferCPU>();

            demoPage(_startDemo, _fontSVGManager);
        }

        void updateCameraUBO()
        {
            _models._camera->computePerspectiveCamera(RT::ViewportInt32(0, 0, _resolution.x, _resolution.y), _sceneBBox);

            _models._screenParameters.projectionMatrix = _models._camera->getProjectionMatrix();
            _models._screenParameters.viewMatrix       = _models._camera->getOrientation().getWorldToLocal().convert();
        }

        void releaseScene()
        {
            // Release resources
            //auto& resources = _core.getResourceManager();

            // Clean buffers
            _models._uboScreenVS->release();
            _models._uboScreenVS.reset();

            _models._ubo2GlyphInfo->release();
            _models._ubo2GlyphInfo.reset();
            _models._ubo2GlyphControls->release();
            _models._ubo2GlyphControls.reset();
            _models._ubo2GlyphPoints->release();
            _models._ubo2GlyphPoints.reset();

            _models._uboTexts->release();
            _models._uboTexts.reset();
        }

        // DisableCodeCoverage

        bool updateUIOverlay(const MouCaGraphic::VulkanManager& manager, bool& refreshCommand)
        {
            bool update = false;

            ImGui::NewFrame();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            ImGui::SetNextWindowPos(ImVec2(10, 10));

            ImGuiIO& io = ImGui::GetIO();
            ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_FirstUseEver);
            ImGui::Begin(u8"Font", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            //ImGui::TextUnformatted(title.c_str());
            //ImGui::TextUnformatted(deviceProperties.deviceName);
            //ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

            ImGui::PushItemWidth(110.0f * 1.0f);
            
            if (ImGui::CollapsingHeader(u8"Demo", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool change = false;
                uint32_t pageId = 0;
                if (ImGui::RadioButton("Basic text", _modeGUI[0]))
                {
                    _modeGUI.fill(false);
                    _modeGUI[0] = true;
                    change = true;
                    pageId = 0;
                }
                if (ImGui::RadioButton("ASCII characters", _modeGUI[1]))
                {
                    _modeGUI.fill(false);
                    _modeGUI[1] = true;

                    change = true;
                    pageId = 1;
                }
                if (ImGui::RadioButton("Unicode", _modeGUI[2]))
                {
                    _modeGUI.fill(false);
                    _modeGUI[2] = true;

                    change = true;
                    pageId = 2;
                }
                if (ImGui::RadioButton("Debug", _modeGUI[3]))
                {
                    _modeGUI.fill(false);
                    _modeGUI[3] = true;

                    change = true;
                    pageId = 3;
                }
                
                if(change)
                {
                    demoPage(pageId, _fontSVGManager);
                    refreshCommand = true;
                }

                update = true;
            }

            if (ImGui::CollapsingHeader(u8"Performances", ImGuiTreeNodeFlags_None))
            {
                static uint32_t nbGlyphs = 1;
                static const uint32_t nbRun = 30; // For stable result
                static double frameMs   = 0.0;
                static double computeMs = 0.0;
                static double appendMs  = 0.0;

                ImGui::Text("Compute font: %.2f ms", computeMs);
                ImGui::Text("  %.2f ms/glyph with %d glyph", computeMs / static_cast<double>(nbGlyphs), nbGlyphs);

                ImGui::Text("Append text:  %.2f ms", appendMs);
                ImGui::Text("  %.2f ms/char  with %d chars", appendMs / static_cast<double>(_nbChars), _nbChars);

                ImGui::Text("Draw:         %.2f ms/frame", frameMs);

                if (ImGui::Button("Compute"))
                {
                    _fontBuild.reset();
                    _charAppends.reset();

                    for (auto run = 0; run < nbRun; ++run)
                    {
                        GUI::FontSVGManager fontManager;
                        demoPage(0, fontManager);
                        nbGlyphs = static_cast<uint32_t>(fontManager._ordered.size());
                    }

                    computeMs = _fontBuild._elapsed   / static_cast<double>(nbRun);
                    appendMs  = _charAppends._elapsed / static_cast<double>(nbRun);

                    frameMs = framePerformance(manager);
                    update = true;
                }                
            }

            ImGui::PopItemWidth();

            ImGui::End();
            ImGui::PopStyleVar();

            ImGui::Render();

            return update;
        }

        double framePerformance(const MouCaGraphic::VulkanManager& manager) const
        {
            auto context = manager.getDevices().at(0);

            // Get Sequencer
            context->getDevice().waitIdle();
            auto queueSequences = context->getQueueSequences();

            const uint32_t nbFrames = 350; // Good number of run
            TimeElapsed timeElapsed;
            
            for(auto frame=0; frame < nbFrames; ++frame)
            {
                timeElapsed.start();
                // Run one frame
                for (const auto& sequence : *queueSequences.at(0))
                {
                    sequence->execute(context->getDevice());
                }
                timeElapsed.cumulate();
            }
            return timeElapsed._elapsed / static_cast<double>(nbFrames);
        }

        // EnableCodeCoverage
};

std::vector<const char*> FontSVGTest::_textDebug =
{
    "L@Q%iE#,"
};

std::vector<const char*> FontSVGTest::_text =
{
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus sit amet scelerisque augue, sit amet commodo neque.",
    "Vestibulum eu eros a justo molestie bibendum quis in urna. Integer quis tristique magna. Morbi in ultricies lorem. Donec",
    "lacinia nisi et arcu scelerisque, eget viverra ante dapibus. Proin enim neque, vehicula id congue quis, consequat sit amet tortor.",
    "Aenean ac lorem sit amet magna rhoncus rhoncus ac ac neque. Cras sed rutrum sem.",
    "Donec placerat ultricies ex, a gravida lorem commodo ut. Mauris faucibus aliquet ligula, vitae condimentum dui semper et.",
    "Aenean pellentesque ac ligula a varius. Suspendisse congue lorem lorem, ac consectetur ipsum condimentum id.",
    "",
    "Vestibulum quis erat sem. Fusce efficitur libero et leo sagittis, ac volutpat felis ullamcorper. Curabitur fringilla eros eget ex",
    "lobortis, at posuere sem consectetur. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis",
    "egestas. Vivamus eu enim leo. Morbi ultricies lorem et pellentesque vestibulum. Proin eu ultricies sem. Quisque laoreet, ligula",
    "non molestie congue, odio nunc tempus arcu, vel aliquet leo turpis non enim. Sed risus dui, condimentum et eros a, molestie",
    "imperdiet nisl. Vivamus quis ante venenatis, cursus magna ut, tincidunt elit. Aenean nisl risus, porttitor et viverra quis,",
    "tempus vitae nisl.",
    "",
    "Suspendisse ut scelerisque tellus. In ac quam sem.Curabitur suscipit massa nisl. Ut et metus sed lacus dapibus molestie. Nullam",
    "porttitor sit amet magna quis dapibus. Nulla tincidunt, arcu sit amet hendrerit consequat, felis leo blandit libero, eu posuere",
    "nisl quam interdum nulla. Quisque nec efficitur libero. Quisque quis orci vitae metus feugiat aliquam eu et nulla. Etiam aliquet",
    "ante vitae lacus aliquam, et gravida elit mollis. Proin molestie, justo tempus rhoncus aliquam, tellus erat venenatis erat,",
    "porttitor dapibus nunc purus id enim. Integer a nunc ut velit porta maximus. Nullam rutrum nisi in sagittis pharetra. Proin id",
    "pharetra augue, sed vulputate lorem. Aenean dapibus, turpis nec ullamcorper pharetra, ex augue congue nibh, condimentum",
    "vestibulum arcu urna quis ex.",
    "",
    "Vestibulum non dignissim nibh, quis vestibulum odio. Ut sed viverra ante, fringilla convallis tellus. Donec in est rutrum,",
    "imperdiet dolor a, vestibulum magna. In nec justo tellus. Ut non erat eu leo ornare imperdiet in sit amet lorem. Nullam quis",
    "nisl diam. Aliquam laoreet dui et ligula posuere cursus.",
    "",
    "Donec vestibulum ante eget arcu dapibus lobortis.Curabitur condimentum tellus felis, id luctus mi ultrices quis. Aenean nulla",
    "justo, venenatis vel risus et, suscipit faucibus nulla. Pellentesque habitant morbi tristique senectus et netus et malesuada",
    "fames ac turpis egestas. Sed lacinia metus eleifend lacinia blandit.Morbi est nibh, dapibus nec arcu quis, volutpat lacinia",
    "dolor. Vestibulum quis viverra erat.Maecenas ultricies odio neque, et eleifend arcu auctor in. Suspendisse placerat massa nisl,",
    "non condimentum ligula sodales at.Phasellus eros urna, elementum in ultricies quis, vulputate id magna. Donec efficitur rutrum",
    "urna sed tempus. Vestibulum eu augue dolor. Vestibulum vehicula suscipit purus, sit amet ultricies ligula malesuada sit amet.",
    "Duis consectetur elit euismod arcu aliquet vehicula. Pellentesque lobortis dui et nisl vehicula, in placerat quam dapibus.Fusce",
    "auctor arcu a purus bibendum, eget blandit nisi lobortis.",
};

std::vector<const char*> FontSVGTest::_textASCII =
{
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "abcdefghijklmnopqrstuvwxyz",
    "0123456789 \t\"'<>[]{}()=_+-*\\/,;:!?.%#&$^@|~"
};

std::vector<const char*> FontSVGTest::_textU8 =
{
    u8"アイウエオ式",
    u8"アイウエオ",
    u8"式指揮士気",
    u8"عندما يريد العالم أن يتكلّم ‬ ، فهو يتحدّث بلغة يونيكود"
};

Core::Path FontSVGTest::_fontRoboto(u8"Roboto-Medium.ttf");

Core::Path FontSVGTest::_fontNoto(u8"NotoSans-Regular.ttf");
Core::Path FontSVGTest::_fontNotoJP(u8"NotoSansCJKjp-Regular.otf"); 
Core::Path FontSVGTest::_fontNotoA(u8"NotoSansArabicUI-Regular.ttf");

Core::Path FontSVGTest::_fontCeltic(u8"Celtic Knot.TTF");
Core::Path FontSVGTest::_fontMevNo(u8"mevno1.ttf");

TEST_F(FontSVGTest, run)
{
    MouCaGraphic::VulkanManager manager;
    MouCaGraphic::ImGUIManager  GUI;

//-------------------------------------------------------------------------------------------------
//                                         Step 1: Build CPU scene
//-------------------------------------------------------------------------------------------------
    makeScene();

//-------------------------------------------------------------------------------------------------
//                                         Step 2: Build GPU scene
//-------------------------------------------------------------------------------------------------

    MouCaGraphic::Engine3DXMLLoader loader(manager);

    // Register CPU memory
    loader._cpuBuffers[0] = _models._uboTexts;
    loader._cpuBuffers[1] = _models._ubo2GlyphInfo;
    loader._cpuBuffers[2] = _models._ubo2GlyphPoints;
    loader._cpuBuffers[3] = _models._ubo2GlyphControls;

    loader._cpuBuffers[4] = _models._specialization;
    loader._cpuBuffers[5] = _models._uboScreenVS;

    loader._cpuMeshDescriptors[0] = getGlyphInstanceDescriptor();

    // Create Vulkan objects
    ASSERT_NO_FATAL_FAILURE(loadEngine(loader, u8"FontSVG.xml"));

    // Link event manager
    loader._dialogs[0].lock()->initialize(_eventManager, _resolution);

    {
        Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loader._commandLinks[0]);
        ASSERT_TRUE(container != nullptr);

        Vulkan::Commands commands;
        commands.emplace_back(std::make_unique<Vulkan::CommandDraw>(4, _glyphCount, 0, 0));
        container->transfer(std::move(commands));

        loader._commandBuffers[0].lock()->execute();
    }

    ASSERT_NO_FATAL_FAILURE(GUI.initialize(_resolution));

    MouCaGraphic::Engine3DXMLLoader loaderGUI(manager);
    // Transfer Device/Framebuffer/RenderPass from first model
    loaderGUI._devices[0]      = loader._devices[0];
    loaderGUI._frameBuffers[0] = loader._frameBuffers[0];
    loaderGUI._renderPasses[0] = loader._renderPasses[0];

    // Register CPU memory
    loaderGUI._cpuMeshDescriptors[0] = GUI.getMeshDescriptor();
    loaderGUI._cpuImages[0]  = GUI.getImageFont();

    loaderGUI._cpuBuffers[0] = GUI.getPush();

    loaderGUI._buffers[0] = GUI.getVertexBuffer();
    loaderGUI._buffers[1] = GUI.getIndexBuffer();

    // Create Vulkan objects
    ASSERT_NO_FATAL_FAILURE(loadEngine(loaderGUI, u8"ImGUI.xml"));

    // Transfer commands
    {
        Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loader._commandLinks[1]);
        ASSERT_TRUE(container != nullptr);
        container->transfer(std::move(*loaderGUI._commandsGroup[0].get()));
    }

//-------------------------------------------------------------------------------------------------
//                                      Step 3: Transfer CPU -> GPU
//-------------------------------------------------------------------------------------------------
    // Get allocated item
    auto context = manager.getDevices().at(0);

    // Transfers data memory to GPU
    {
        auto pool = std::make_shared<Vulkan::CommandPool>();
        pool->initialize(context->getDevice(), context->getDevice().getQueueFamilyGraphicId());

        Vulkan::CommandBufferSPtr commandBuffer = std::make_shared<Vulkan::CommandBuffer>();
        commandBuffer->initialize(context->getDevice(), pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);

        MouCaGraphic::Engine3DTransfer transfer(*context);

        // Buffers
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[0].lock(), *loader._buffers[0].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[1].lock(), *loader._buffers[1].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[2].lock(), *loader._buffers[2].lock());
        transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[3].lock(), *loader._buffers[3].lock());

        // GUI
        transfer.indirectCopyCPUToGPU(commandBuffer, *loaderGUI._cpuImages[0].lock(), *loaderGUI._images[0].lock());
            
        // Go
        transfer.transfer(commandBuffer);

        // Release
        commandBuffer->release(context->getDevice());
        pool->release(context->getDevice());
    }

    updateCameraUBO();
    {
        MouCaGraphic::Engine3DTransfer transfer(*context);
        transfer.immediatCopyCPUToGPU(*_models._uboScreenVS, *loader._buffers[5].lock());
    }

//-------------------------------------------------------------------------------------------------
//                                             Step 4: Play
//-------------------------------------------------------------------------------------------------
    bool refreshCommand = false;
    // Execute rendering
    if (MouCaEnvironment::isDemonstrator())
    {
        // DisableCodeCoverage

        updateUIOverlay(manager, refreshCommand);

        bool needUpdateGUI = true;

        /// Update Light position / camera
        auto demo = [&](const double )
        {
            // Move camera
            {
                updateCameraUBO();
                MouCaGraphic::Engine3DTransfer transfer(*context);
                transfer.immediatCopyCPUToGPU(*_models._uboScreenVS, *loader._buffers[5].lock());
            }

            // Update mouse position into ImGUI
            GUI.updateMouse(_graphic.getRTPlatform().getMouse());
            
            refreshCommand = false;

            if (updateUIOverlay(manager, refreshCommand) || needUpdateGUI)
            {
                needUpdateGUI = false;

                ASSERT_NO_THROW(GUI.prepareBuffer(*context));
                Vulkan::CommandContainer* containerGUI = dynamic_cast<Vulkan::CommandContainer*>(loaderGUI._commandLinks[0]);
                ASSERT_TRUE(containerGUI != nullptr);
                ASSERT_NO_THROW(GUI.buildCommands(containerGUI->getCommands()));

                if (refreshCommand)
                {
                    // Strong sync
                    context->getDevice().waitIdle();

                    auto pool = std::make_shared<Vulkan::CommandPool>();
                    pool->initialize(context->getDevice(), context->getDevice().getQueueFamilyGraphicId());

                    Vulkan::CommandBufferSPtr commandBuffer = std::make_shared<Vulkan::CommandBuffer>();
                    commandBuffer->initialize(context->getDevice(), pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);

                    // Reallocation of buffer if mandatory
                    {
                        bool reallocation = false;
                        for(uint32_t id=0; id < 4; ++id)
                        {
                            auto bufferGPU = loader._buffers[id].lock();
                            auto bufferCPU = loader._cpuBuffers[id].lock();
                            if (bufferGPU->getSize() < bufferCPU->getByteSize())
                            {
                                bufferGPU->resize(context->getDevice(), bufferCPU->getByteSize());
                                reallocation = true;
                            }
                        }

                        if(reallocation)
                        {
                            loader._descriptorSets[1].lock()->update(context->getDevice());
                        }
                    }

                    // Update data
                    MouCaGraphic::Engine3DTransfer transfer(*context);
                    transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[0].lock(), *loader._buffers[0].lock());
                    transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[1].lock(), *loader._buffers[1].lock());
                    transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[2].lock(), *loader._buffers[2].lock());
                    transfer.indirectCopyCPUToGPU(commandBuffer, *loader._cpuBuffers[3].lock(), *loader._buffers[3].lock());

                    // Go
                    transfer.transfer(commandBuffer);

                    // Release
                    commandBuffer->release(context->getDevice());
                    pool->release(context->getDevice());

                    {
                        Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loader._commandLinks[0]);
                        container->clear();
                        Vulkan::Commands commands;
                        commands.emplace_back(std::make_unique<Vulkan::CommandDraw>(4, _glyphCount, 0, 0));
                        container->transfer(std::move(commands));

                        loader._commandBuffers[0].lock()->execute();
                    }
                }

                // Refresh Command Buffer with new command
                loader._surfaces[0].lock()->updateCommandBuffer(0);
            }
        };
        mainLoop(manager, u8"FontSVG Demo ", demo);

        // EnableCodeCoverage
    }
    else
    {
        updateUIOverlay(manager, refreshCommand);
        // Build GUI design
        updateUIOverlay(manager, refreshCommand);
        ASSERT_NO_THROW(GUI.prepareBuffer(*context));
        Vulkan::CommandContainer* container = dynamic_cast<Vulkan::CommandContainer*>(loaderGUI._commandLinks[0]);
        ASSERT_TRUE(container != nullptr);
        ASSERT_NO_THROW(GUI.buildCommands(container->getCommands()));

        // Refresh Command Buffer with new command
        loader._surfaces[0].lock()->updateCommandBuffer(0);

        // Get Sequencer
        context->getDevice().waitIdle();
        auto queueSequences = context->getQueueSequences();
        ASSERT_EQ(1, queueSequences.size());
        
        // Run one frame
        for (const auto& sequence : *queueSequences.at(0))
        {
            ASSERT_EQ(VK_SUCCESS, sequence->execute(context->getDevice()));
        }

        // Compare to expected result
        takeScreenshot(manager, L"VulkanFontSVG.png");
    }

    // Clean
    ASSERT_NO_THROW(GUI.release(*context));
    ASSERT_NO_THROW(manager.release());
    ASSERT_NO_THROW(releaseScene());

    // Clean allocate dialog
    ASSERT_NO_THROW(clearDialog(manager));
    ASSERT_NO_THROW(releaseEventManager());
}