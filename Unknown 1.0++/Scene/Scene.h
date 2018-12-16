//
// Created by cub3d on 01/06/17.
//

#ifndef UNKNOWN_1_0_CPP_SCENE_H
#define UNKNOWN_1_0_CPP_SCENE_H


#include <string>
#include <functional>
#include <Box2D/Box2D.h>

#include <UI.h>
#include <Font/Font.h>
#include <Renderer/IRenderable.h>
#include <IUpdateable.h>
#include <Renderer/Camera.h>
#include <Entity/Entity.h>
#include "CollisionManager.h"
#include <Scene/SceneGraph.h>
#include <Settings/SettingsParser.h>
#include <Loader.h>
#include <Entity/Component.h>

namespace Unknown
{
    class Entity;
    class CollisionManager;
    class Component;

    class Scene
    {
    public:
        std::vector<std::shared_ptr<IRenderable>> renderables;
        std::vector<std::shared_ptr<IUpdateable>> updatables;
        std::vector<std::shared_ptr<ITagable>> tagables;
        std::vector<std::shared_ptr<Entity>> entities;
        b2World world;
        CollisionManager contactManager;
        Camera cam;


        Scene();

        virtual void render() const;
        virtual void update();
        virtual void reset();

        template<class T>
        void addObject(std::shared_ptr<T> obj) {
            if(dynamic_cast<IRenderable*>(obj.get())) {
                renderables.push_back(std::dynamic_pointer_cast<IRenderable>(obj));
            }
            if(dynamic_cast<IUpdateable*>(obj.get())) {
                updatables.push_back(std::dynamic_pointer_cast<IUpdateable>(obj));
            }
            if(dynamic_cast<ITagable*>(obj.get())) {
                tagables.push_back(std::dynamic_pointer_cast<ITagable>(obj));
            }
            if(dynamic_cast<Entity*>(obj.get())) {
                auto ent = std::dynamic_pointer_cast<Entity>(obj);

                for(std::shared_ptr<Component>& comp : ent->prototype.components) {
                    comp->onEnable(*this, ent);
                }

                entities.push_back(ent);
            }
        }

        void registerEntityCollision(std::shared_ptr<Entity> ent1, std::shared_ptr<Entity> ent2, std::function<void(std::pair<std::shared_ptr<Entity>, std::shared_ptr<Entity>>, bool)> callback);

        void registerEntityCollision(const std::string& ent1, const std::string& ent2, std::function<void(std::pair<std::shared_ptr<Entity>, std::shared_ptr<Entity>>, bool)> callback);

        std::shared_ptr<Entity> getEntity(const std::string& name);

        void loadScenegraph(const std::string& name);


        template<class T>
        std::shared_ptr<T> getObject(const std::string& str) {
            for (auto& tagable : this->tagables) {
                if(tagable->getTag() == str) {
                    if (dynamic_cast<T *>(tagable.get())) {
                        return std::dynamic_pointer_cast<T>(tagable);
                    }
                }
            }

            return nullptr;
        }

        template<class T>
        const std::shared_ptr<T> getObject(const std::string& str) const {
            for (auto& tagable : this->tagables) {
                if(tagable->getTag() == str) {
                    if (dynamic_cast<T *>(tagable.get())) {
                        return std::dynamic_pointer_cast<T>(tagable);
                    }
                }
            }

            return nullptr;
        }

        template<typename T>
        std::vector<std::shared_ptr<T>> getObjects(const std::string& tag) const {
            std::vector<std::shared_ptr<T>> objs;
            for (auto& tagable : this->tagables) {
                if(tagable->getTag() == tag) {
                    if (dynamic_cast<T *>(tagable.get())) {
                        objs.push_back(std::dynamic_pointer_cast<T>(tagable));
                    }
                }
            }

            return objs;
        }
    };

    class MenuScene : public Scene
    {
        std::string uiFile;
        std::shared_ptr<Graphics::Font> font;

    public:
        UIContainer menu;

        MenuScene(std::string uiFile, std::shared_ptr<Graphics::Font> font);

        virtual void render() const override;
        virtual void update() override;

        void reloadMenu();
    };

    class CustomScene : public Scene
    {
        const std::function<void(void)> renderer;
        const std::function<void(void)> updater;

    public:
        CustomScene(std::function<void(void)> renderer, std::function<void(void)> updater);
        virtual void render() const override;
        virtual void update() override;
    };
}


#endif //UNKNOWN_1_0_CPP_SCENE_H
