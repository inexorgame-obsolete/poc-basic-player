#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Primitives/Axis.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/MeshData3D.h>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace Magnum;
using namespace Magnum::Math::Literals;

/*
TODO:
 - [DONE] Camera matrix richtig
 - [DONE] Keys WASD
 - [DONE] fix camera resetting
 - walk in camera direction
 - intermediate frames
 - add coordinate systems (local + global)

 - Octree Datastructure
 - Add / Remove Octree Datenstructure
 - Convert Octree -> Mesh
 - Select Octree Cube
 - Update Mesh wenn Add / Remove
 */

/// Dummy for collision checks
bool collide(const Vector3 &position)
{
    // TODO: get entity size in here, so collision can be checked..
    if (position.y() <= -3.0f)
        return true;
    return false;
}

/// Walk for a certain distance in a certain directory (if no collision occurs).
/// @param dir: the directory vector to move to.
/// @param dist: the distance to move
/// @param stepdist: the movement is subdivided in certain steps. If the distance
///                  to be travelled is below this threshold, no action is taken.
///                  Also collision is checked for each step, not only after completing
///                  the whole distance.
/// @param[in,out] position: the position to be updated
/// @return: true if move was successful, false if a collision occured inbetween.
bool move(Vector3 &position, Vector3 dir, float dist, float stepdist)
{
    int steps = (int)ceil(dist/stepdist);
    if(steps <= 0) return true;

    dir *= dist/steps;
    for(int i = 0; i < steps; i++)
    {
        Vector3 old_position(position);
        position += dir;
        if(collide(position))
        {
            position = old_position;
            return false;
        }
    }
    return true;
}

struct Camera {
    /// World coordinates (x, y, z) where the camera is located.
    Vector3 position;
    /// Yaw (around local z), Pitch (around local x), Roll (around local y) in degree.
    Vector3 rotation; // TODO: unused
    Rad yaw, pitch, roll;

    /// Stays the same as long as
    Matrix4 _projection_matrix;

    Camera(float aspect_ratio) {
        // constexpr float eyeheight = 18;
        constexpr float eyeheight = 18;
        position = Vector3(0, 0, -1);
        _projection_matrix =
                Matrix4::perspectiveProjection(
                        35.0_degf, aspect_ratio, 0.01f, 100.0f)*
                Matrix4::translation(Vector3::zAxis(-10.0f));
    }

    /// Return the matrix which transforms the world coordinates to coordinates
    /// in the camera local coordination frame.
    Matrix4 get_transformation_matrix() {
        return Matrix4::rotationY(-roll)
               * Matrix4::rotationX(pitch)
               * Matrix4::rotationZ(yaw)
               * Matrix4::translation(position);
    }

    /// Return a rotation matrix which translates each vector to be rotated in
    /// to the viewing direction.
    /// Used e.g. if you want to get the vector which is in forward direction of
    /// the camera.
    Matrix3x3 get_orientation_matrix() {
        const auto rotation_translation_matrix = Matrix4::rotationY(-roll)
                                           * Matrix4::rotationX(pitch)
                                           * Matrix4::rotationZ(yaw);

        return rotation_translation_matrix.rotation();
    }

    /// Return the matrix which transforms 3D coordinates to 2D coordinates
    /// based on the camera's fov and the window size.
    Matrix4 get_projection_matrix() {
        return _projection_matrix;
    }

    /// Rotate the camera for certain angles around the local Z (Yaw) and X (Pitch) axes.
    void reorientate(const Rad &delta_yaw, const Rad &delta_pitch);
};

/// reset yaw and pitch, otherwise continously spinning around the own axis overflows them.
void fix_camera_range(Camera &camera)
{
/*
    const Rad MAXPITCH = 90.0_degf;
    if(camera.pitch>MAXPITCH) camera.pitch = MAXPITCH;
    if(camera.pitch<-MAXPITCH) camera.pitch = -MAXPITCH;
    while(camera.yaw<0.0_degf) camera.yaw += 360.0_degf;
    while(camera.yaw>=360.0_degf) camera.yaw -= 360.0_degf;
 TODO: maybe this is already handled by the type wrappers?
 */
}
void Camera::reorientate(const Rad &delta_yaw, const Rad &delta_pitch) {
    yaw += delta_yaw;
    pitch += delta_pitch;
    fix_camera_range(*this);
}

/// A class to contain the Meshes and data to be rendered by a Renderer.
struct Renderable {
    GL::Buffer _indexBuffer, _vertexBuffer;
    GL::Mesh _mesh;
    Color3 _color;
    Shaders::Phong _shader;
    // GL::AbstractShaderProgram _shader;
    /*
     * problem: each renderable may need another shader..
     * so maybe make it a link?
     * wohin constructor für beides?
     * wohin draw functions? draw(camera, shader)
     */
};

Renderable create_world() {
    Renderable w;

    const Trade::MeshData3D cube = Primitives::cubeSolid();
    w._vertexBuffer.setData(MeshTools::interleave(cube.positions(0), cube.normals(0)));

    Containers::Array<char> indexData;
    MeshIndexType indexType;
    UnsignedInt indexStart, indexEnd;
    std::tie(indexData, indexType, indexStart, indexEnd) =
            MeshTools::compressIndices(cube.indices());
    w._indexBuffer.setData(indexData);

    w._mesh.setPrimitive(cube.primitive())
            .setCount(cube.indices().size())
            .addVertexBuffer(w._vertexBuffer, 0, Shaders::Phong::Position{},
                             Shaders::Phong::Normal{})
            .setIndexBuffer(w._indexBuffer, 0, indexType, indexStart, indexEnd);

    w._color = Color3::fromHsv({35.0_degf, 1.0f, 1.0f});
    w._shader.setLightPosition({7.0f, 5.0f, 2.5f})
            .setLightColor(Color3{1.0f})
            .setDiffuseColor(w._color)
            .setAmbientColor(Color3::fromHsv({w._color.hue(), 1.0f, 0.3f}));
    return std::move(w);
}

class PrimitivesExample: public Platform::Application {
public:
    explicit PrimitivesExample(const Arguments& arguments);

private:
    void drawEvent() override;
    void mouseMoveEvent(MouseMoveEvent& event) override;
    void keyPressEvent(KeyEvent& event) override;

    Camera camera;
    /// everything which is to be displayed: world geometry, editing helper geometry, ..
    std::vector<Renderable> renderables;
    std::unordered_map<int, GL::AbstractShaderProgram> shaders;
    Vector2i _previousMousePosition;
};

PrimitivesExample::PrimitivesExample(const Arguments& arguments):
        Platform::Application{arguments, Configuration{}.setTitle("Magnum Primitives Example")},
        camera(Vector2{windowSize()}.aspectRatio())
{
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    // Geometry: position is -1 to 1
    renderables.push_back(create_world());
    //renderables.push_back(Renderable(Primitives::axis3D()));
}

void PrimitivesExample::drawEvent() {
    GL::defaultFramebuffer.clear(
            GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    const Matrix4 _projection = camera.get_projection_matrix();
    const Matrix4 _transformation = camera.get_transformation_matrix();
    for (auto &r : renderables)
    {
        r._shader.setTransformationMatrix(_transformation)
                .setNormalMatrix(_transformation.rotationScaling())
                .setProjectionMatrix(_projection);
        r._mesh.draw(r._shader);
    }

    swapBuffers();
}

void PrimitivesExample::keyPressEvent(KeyEvent& event) {
    // code from recompute_camera in sauerbraten.

    // where do we look?
    auto orientation = camera.get_orientation_matrix();
    Vector3 forward_dir = -orientation[1];
    Vector3 side_dir = -orientation[0];
    Vector3 up_dir = orientation[2];
    switch(event.key()) {
        case KeyEvent::Key::A: move(camera.position, side_dir, 1, 1); break;
        case KeyEvent::Key::D: move(camera.position, -side_dir, 1, 1); break;
        case KeyEvent::Key::W: move(camera.position, forward_dir, 1, 1); break;
        case KeyEvent::Key::S: move(camera.position, -forward_dir, 1, 1); break;
    }
    event.setAccepted();
    redraw();
}

void PrimitivesExample::mouseMoveEvent(MouseMoveEvent& event) {
    // if(!(event.buttons() & MouseMoveEvent::Button::Left)) return;

    const Vector2 delta = 3.0f*
                          Vector2{event.position() - _previousMousePosition}/
                          Vector2{GL::defaultFramebuffer.viewport().size()};

    float mouse_sensitivity = 10.0f;
    float mouse_sensitivity_scale = 100.0f;

    float real_sensitivity = mouse_sensitivity / mouse_sensitivity_scale;

    // camera.reorientate(Rad(delta.x() * real_sensitivity), Rad(delta.y() * real_sensitivity));
    camera.reorientate(Rad(delta.x()), Rad(delta.y()));
    _previousMousePosition = event.position();
    event.setAccepted();
    redraw(); // TODO: remove and make continously draw..
}

MAGNUM_APPLICATION_MAIN(PrimitivesExample)