// Unknown Test.cpp : Defines the entry point for the console application.
//

#include <UK.h>

#include <iostream>

#include <cmath>
#include <memory>
#include <SDL_mixer.h>
#include <Editor/EditorBase.h>
#include <SDL_rwops.h>
#include <LibPAK.h>

#include "PhysicsTestScene.h"
#include "RenderTestScene.h"
#include "RenderTest2D.h"
#include "PathfindingTest.h"
#include "GL/GL.h"

#include "UK.h"

//void UICallback(std::shared_ptr<Unknown::UIEvent> evnt)
//{
//    if(evnt->componentName == "ButtonStart")
//    {
//        Unknown::UIContainer& mainMenu = Unknown::getUnknown().globalSceneManager.getScene<Unknown::MenuScene>()->menu;
//        std::string widthText = mainMenu.getComponentValue("TextBoxWidth");
//        std::string heightText = mainMenu.getComponentValue("TextBoxHeight");
//        std::string speedText = mainMenu.getComponentValue("TextBoxSpeed");
//
//        int boardWidth = 0;
//        int boardHeight = 0;
//        int speed = 0;
//
//        if(Unknown::stringToInt(widthText, boardWidth) && Unknown::stringToInt(heightText, boardHeight)) {
//            printf("Creating board with size %dx%d\n", boardWidth, boardHeight);
//            width = boardWidth;
//            height = boardHeight;
//            UK_LOAD_SCENE("Simulator");
//            createBoard();
//            //TODO: create a resize function under unknown
//            //SDL_SetWindowSize(Unknown::getUnknown()->window, width*pixelScale, height*pixelScale);
//        }
//
//        if(Unknown::stringToInt(speedText, speed))
//        {
//            timer.timerSpeed = speed;
//        }
//    }
//}
//
//void onResize(Unknown::Event& evt) {
//    if(Unknown::getUnknown().globalSceneManager.currentSceneName == "Simulator") {
//        auto& uk = Unknown::getUnknown();
//        scaleX = uk.screenSize->width / width;
//        scaleY = uk.screenSize->height / height;
//    } else {
//        auto menu = dynamic_cast<Unknown::MenuScene*>(Unknown::getUnknown().globalSceneManager.currentScene.get());
//        if(menu) {
//            menu->reloadMenu();
//        }
//    }
//}

void render() {
    ImGui::Begin("Test");

    ImGui::Text("Pre tex %d", glGetError());
    std::string _ = "Player.png";
    static Unknown::TextureInfo t = Unknown::getRendererBackend()->loadTexture(_);

    ImGui::Text("Post tex %d", glGetError());

    static std::string vert = "#version 300 es\n"
                                                     "\n"
                                                     "precision highp float;\n"
                                                     "layout(location = 0) in vec4 inVertex;\n"
                                                     "layout(location = 1) in vec2 inUV;\n"
                                                     "uniform mat4 mvp;\n"
                                                     "out vec2 UV;\n"
                                                     "\n"
                                                     "void main() {\n"
                                                     "    UV = inUV;\n"
                                                     "    gl_Position = mvp * inVertex;\n"
                                                     "}";

    static std::string frag = "#version 300 es\n"
                                                       "\n"
                                                       "precision highp float;\n"
                                                       "\n"
                                                       "uniform sampler2D tex;\n"
                                                       "out vec4 fragColour;\n"
                                                       "in vec2 UV;\n"
                                                       "\n"
                                                       "void main() {\n"
                                                       "    fragColour = vec4(vec3(texture(tex, UV)), 1.0f);\n"
                                                       "}";

    float screenWidth = 1024;
    float screenHeight = 1024;

    static glm::mat4 proj = glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, 0.0f, 1.0f);
    static glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, 0.0f, 0.0f));
    static glm::mat4 mvp = proj * model;

    static Shader s(vert, frag);

    constexpr const int SIZE = 6 * (3 + 2);

    int w = 100, h = 100;

   static GLuint vao = -1;
   static GLuint vbo = -1;

    ImGui::Text("Precreate %d", glGetError());

   if(vao == -1) {
       GLfloat data[SIZE]{
           // Format is vertex coord
           // Then colour
           // Then texcoord
           // Then normal
           0, 0, 0,
           0, 0,
           (GLfloat) w, 0, 0,
           1, 0,
           (GLfloat) w, (GLfloat) h, 0,
           1, 1,

           0, 0, 0,
           0, 0,
           0, (GLfloat) h, 0,
           0, 1,
           (GLfloat) w, (GLfloat) h, 0,
           1, 1,
       };

       glGenBuffers(1, &vbo);

       // Bind VBO and put data in it
       glBindBuffer(GL_ARRAY_BUFFER, vbo);
       glBufferData(GL_ARRAY_BUFFER, SIZE * sizeof(GLfloat), data, GL_STATIC_DRAW);


       glGenVertexArrays(1, &vao);

       // Bind the VAO and fill in the locations of each piece of vertex data
       glBindVertexArray(vao);
       constexpr const int stride = (3 + 2) * sizeof(GLfloat); // Size of each sub block

       // Verticies
       glEnableVertexAttribArray(0);
       glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

       glEnableVertexAttribArray(1);
       glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(GLfloat)));
   }

    ImGui::Text("Rendering %d", glGetError());
    s.bind(true);

    glUniformMatrix4fv(glGetUniformLocation(s.prog, "mvp"), 1, GL_FALSE, &mvp[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, (GLuint) t.pointer);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    s.unbind();

    ImGui::Text("Error: %d", glGetError());

    ImGui::End();
}

void init()
{
//    UK_ADD_UI_LISTENER_EXTERNAL(UICallback, "mainmenu");
//
//    Unknown::registerEventHandler(Unknown::ET_WINDOW_RESIZE, "onResize", onResize);
//
//	tmp.image = new Unknown::Graphics::Image("Player.png");
//
//    font = std::make_shared<Unknown::Graphics::TTFont>("Fonts/Arimo-Regular.ttf", 14, Unknown::Colour::BLACK);

    //UK_ADD_SCENE(std::make_shared<Unknown::MenuScene>("MainMenu", "MainMenuUI.json", font));

    UK_ADD_SCENE(RenderTestScene, "RTest");
//    UK_ADD_SCENE(RenderTest2D, "R2D");
//    UK_ADD_SCENE(PhysicsTestScene, "Phys");
//    UK_ADD_SCENE(BasicGraphicsTest, "BasicGraphics");
    UK_LOAD_SCENE("RTest");
}

#include "Filesystem/Filesystem.h"

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	UK_CREATE_WINDOW();
//
//    char *source = NULL;
//    int bufsize;
//    FILE *fp = fopen("Player.png", "r");
//    if (fp != NULL) {
//        /* Go to the end of the file. */
//        if (fseek(fp, 0L, SEEK_END) == 0) {
//            /* Get the size of the file. */
//            bufsize = ftell(fp);
//            if (bufsize == -1) { /* Error */ }
//
//            /* Allocate our buffer to that size. */
//            source = static_cast<char *>(malloc(sizeof(char) * bufsize));
//
//            /* Go back to the start of the file. */
//            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }
//
//            /* Read the entire file into memory. */
//            size_t newLen = fread(source, sizeof(char), bufsize, fp);
//            if ( ferror( fp ) != 0 ) {
//                fputs("Error reading file", stderr);
//            }
//        }
//        fclose(fp);
//    }
//
//
//    PAKFile file;
//	initPAK(&file, 1, 90000);
//    auto ent = addFile(&file, bufsize, "/test.png");
//    printf("Size %d\n", ent->fileSize);
//    setFileData(&file, ent, source, bufsize);
//    printf("finalizing\n");
//    finallizePAK(&file);
//	writePak(file);
//
//    free(source); /* Don't forget to call free() later! */
//


    ::Unknown::Filesystem::mount("Test.pak");

    // Test the loading of vectors

	init();
	UK_INIT_GAME();

	return 0;
}
