//
// Created by cub3d on 01/06/17.
//
#include "Scene.h"
#include <Loader.h>
#include <Font/Font.h>
#include <core/log/Log.h>

Unknown::Scene::Scene() : world(b2Vec2(0, 9.8f)),
cam(getUnknown().screenSize.x, getUnknown().screenSize.y)
{
    world.SetContactListener(&contactManager);
}

void Unknown::Scene::update()
{
    this->world.Step(1.0f/60.0f, 8, 3);
   //this->world.Step(getUnknown().tickSpeed / 1000, 8, 3);

   for(auto& updateable : this->updatables) {
       if(updateable)
           updateable->update();
   }
}

void Unknown::Scene::render() const {
    for(auto& renderable : this->renderables) {
        if(!renderable)
            continue;
        // Only render things that are in the view of the camera
        if(cam.getBounds().contains(renderable->getRenderBounds()) || true) {
            //TODO: pass some offset
            renderable->render(cam.area.x, cam.area.y);
        }
    }
}

std::shared_ptr<Unknown::Entity> Unknown::Scene::getEntity(const std::string &name) {
    return getObject<Entity>(name);
}

void Unknown::Scene::loadScenegraph(const std::string &name) {
    auto prototype = SettingsParser::parseSettings<SceneGraph>(name);

    for(auto&& element : prototype.sceneElements) {
        auto entityInstance = Loader::loadEntityAt(element.name, element.position.x, element.position.y);
        this->addObject(entityInstance);
    }

    UK_INFO("Loaded", std::to_string(prototype.sceneElements.size()), "scene:", name);
}

void Unknown::Scene::reset() {
    auto& ss = getUnknown().screenSize;

    this->updatables.clear();
    this->tagables.clear();
    this->renderables.clear();
    this->entities.clear();
    this->contactManager.reset();

    UK_INFO("World locked? {}", world.IsLocked());

//    for(auto b = world.GetBodyList(); b;) {// b->GetNext()) {
//        auto n = b->GetNext();
//        world.DestroyBody(b);
//        b = n;
//    }


    //TODO: reset camera, sometimes crashes
}

void Unknown::Scene::registerEntityCollision(std::shared_ptr<Entity> ent1,
                                             std::shared_ptr<Entity> ent2,
                                             std::function<void(std::pair<std::shared_ptr<Entity>, std::shared_ptr<Entity>>, bool)> callback) {
    this->contactManager.addListener(ent1, ent2, callback);
}

void Unknown::Scene::registerEntityCollision(const std::string &ent1,
                                             const std::string &ent2,
                                             std::function<void(std::pair<std::shared_ptr<Entity>, std::shared_ptr<Entity>>, bool)> callback) {
    registerEntityCollision(getEntity(ent1), getEntity(ent2), callback);
}

Unknown::MenuScene::MenuScene(std::string uiFile, std::shared_ptr<Font> font) : Scene(), uiFile(uiFile), font(font)
{
    if(!uiFile.empty()) {
        this->menu = Loader::loadUI(uiFile);
    } else {
        this->menu = UIContainer();
    }
    this->menu.setGlobalFont(font);
    this->menu.initUI();
}

void Unknown::MenuScene::render() const
{
    this->menu.render(0, 0);
    Scene::render();
}

void Unknown::MenuScene::update()
{
    Scene::update();
}

void Unknown::MenuScene::reloadMenu() {
    // TODO: quick and dirty for getting resizing ui's, make a way to recalculate offsets inplace
    this->menu = Loader::loadUI(this->uiFile);
    this->menu.setGlobalFont(this->font);
    this->menu.initUI();
}

Unknown::CustomScene::CustomScene(std::function<void(void)> renderer, std::function<void(void)> updater) : Scene(), renderer(renderer), updater(updater)
{
}

void Unknown::CustomScene::render() const
{
    if(this->renderer)
        this->renderer();
    Scene::render();
}

void Unknown::CustomScene::update()
{
    if(this->updater)
        this->updater();
    Scene::update();
}