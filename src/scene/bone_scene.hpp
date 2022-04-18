#ifndef SRC_SCENE_BONE_SCENE_H
#define SRC_SCENE_BONE_SCENE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <json/json.h>

#include "glcpp/cubemap.h"
#include "glcpp/shader.h"
#include "glcpp/model.h"
#include "glcpp/camera.h"
#include "glcpp/framebuffer.h"
#include "glcpp/transform_component.h"
#include "scene.hpp"

#include <map>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

// simple rendering
class BoneScene : public Scene
{
public:
    BoneScene(int width, int height)
    {
        init_skybox();
        init_shader();
        init_model();
        init_framebuffer(width, height);
        init_camera();
    }
    virtual ~BoneScene() = default;
    virtual void init_framebuffer(uint32_t width, uint32_t height) override
    {
        set_size(width, height);
        framebuffer_.reset(new glcpp::Framebuffer{width, height, GL_RGB});
    }
    virtual void add_model(const char *file_name) override
    {
        model_.reset(new glcpp::Model{file_name});
    }
    virtual std::shared_ptr<glcpp::Model> &get_model() override
    {
        return model_;
    }
    virtual std::shared_ptr<glcpp::Framebuffer> &get_framebuffer() override
    {
        return framebuffer_;
    }
    virtual void pre_draw() override
    {
        if (framebuffer_->get_width() != width_ || framebuffer_->get_height() != height_)
        {
            init_framebuffer(width_, height_);
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera_->Zoom), framebuffer_->get_aspect(), 0.1f, 10000.0f);
        glm::mat4 view = camera_->GetViewMatrix();

        glEnable(GL_DEPTH_TEST);
        framebuffer_->bind_with_depth();
        {
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            model_->draw(*model_shader_, view, projection);
            skybox_->draw(view, projection);
        }

        framebuffer_->unbind();
    }
    virtual void draw() override
    {
        framebuffer_->draw(*framebuffer_shader_);
    }
    virtual std::shared_ptr<glcpp::Camera> &get_camera() override
    {
        return camera_;
    }
    virtual void set_size(uint32_t width, uint32_t height) override
    {
        width_ = width;
        height_ = height;
    }
    virtual void print_to_png(const std::string &file_name) override {}

private:
    void init_skybox()
    {
        skybox_ = std::make_unique<glcpp::Cubemap>(skybox_faces_[1],
                                                   "./../../resources/shaders/skybox.vs",
                                                   "./../../resources/shaders/skybox.fs");
    }
    void init_shader()
    {
        model_shader_ = std::make_unique<glcpp::Shader>("./../../resources/shaders/1.model_loading.vs", "./../../resources/shaders/1.model_loading.fs");
        framebuffer_shader_ = std::make_unique<glcpp::Shader>("./../../resources/shaders/simple_framebuffer.vs", "./../../resources/shaders/simple_framebuffer.fs");
    }
    void init_model()
    {
        add_model(fs::canonical(fs::path("./../../resources/models/nanosuit/nanosuit.obj")).string().c_str());
        model_->get_mutable_transform().set_translation(glm::vec3{0.0f, 0.0f, 0.0f}).set_rotation(glm::vec3{0.0f, 0.0f, 0.0f}).set_scale(glm::vec3{1.0f, 1.0f, 1.0f});
    }
    void init_camera()
    {
        camera_ = std::make_shared<glcpp::Camera>(glm::vec3(0.0f, 0.0f, 20.0f));
    }

private:
    // skybox
    std::vector<std::string> skybox_faces_[4]{
        {"./../../resources/textures/skybox/right.jpg",
         "./../../resources/textures/skybox/left.jpg",
         "./../../resources/textures/skybox/top.jpg",
         "./../../resources/textures/skybox/bottom.jpg",
         "./../../resources/textures/skybox/front.jpg",
         "./../../resources/textures/skybox/back.jpg"},
        {"./../../resources/textures/cube/Bridge2/px.jpg",
         "./../../resources/textures/cube/Bridge2/nx.jpg",
         "./../../resources/textures/cube/Bridge2/py.jpg",
         "./../../resources/textures/cube/Bridge2/ny.jpg",
         "./../../resources/textures/cube/Bridge2/pz.jpg",
         "./../../resources/textures/cube/Bridge2/nz.jpg"},
        {"./../../resources/textures/cube/MilkyWay/px.jpg",
         "./../../resources/textures/cube/MilkyWay/nx.jpg",
         "./../../resources/textures/cube/MilkyWay/py.jpg",
         "./../../resources/textures/cube/MilkyWay/ny.jpg",
         "./../../resources/textures/cube/MilkyWay/pz.jpg",
         "./../../resources/textures/cube/MilkyWay/nz.jpg"},
        {"./../../resources/textures/cube/pisa/px.png",
         "./../../resources/textures/cube/pisa/nx.png",
         "./../../resources/textures/cube/pisa/py.png",
         "./../../resources/textures/cube/pisa/ny.png",
         "./../../resources/textures/cube/pisa/pz.png",
         "./../../resources/textures/cube/pisa/nz.png"}};

    std::unique_ptr<glcpp::Cubemap> skybox_;
    std::shared_ptr<glcpp::Model> model_;
    std::shared_ptr<glcpp::Shader> model_shader_;
    std::shared_ptr<glcpp::Shader> framebuffer_shader_;
    std::shared_ptr<glcpp::Camera> camera_;
    std::shared_ptr<glcpp::Framebuffer> framebuffer_;
    uint32_t width_ = 800;
    uint32_t height_ = 600;
};

#endif