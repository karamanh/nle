/**
 * @file terrain_3d.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Ground: a grid mesh you can stand on, pick with the mouse and decorate.
 * @version 0.1
 * @date 2026-09-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "../core/ray.h"
#include "../mesh/mesh_instance_3d.h"

#include <functional>
#include <vector>

namespace nle
{

class model;

/**
 * @brief One stop of the terrain's colour ramp.
 *
 * Between two stops the colour is interpolated, so a handful of these is
 * enough to get sand into grass into rock into snow.
 */
struct terrain_layer
{
    /// Height, in the terrain's local space, at which this colour is reached.
    float height = 0.0f;

    glm::vec3 color = glm::vec3(1.0f);
};

/**
 * @brief Fractal noise settings for a generated heightfield.
 *
 * The defaults give gentle, rolling ground rather than mountains, on the
 * assumption that something has to walk over it.
 */
struct terrain_noise
{
    int seed = 1337;

    /// Feature size, per world unit. Smaller is broader.
    float frequency = 0.02f;

    int octaves = 4;
    float lacunarity = 2.0f;
    float gain = 0.5f;

    /// Peak height above the base. The field spans roughly [-amplitude, amplitude].
    float amplitude = 6.0f;

    /// A level circle at the centre, so there is somewhere to spawn and build.
    /// Zero disables it.
    float flat_radius = 0.0f;

    /// Distance over which that circle blends back into the hills.
    float flat_falloff = 12.0f;

    /// Pulls the outside edge of the patch down to the base height, so the map
    /// ends in a rim instead of a cliff. Zero disables it.
    float edge_falloff = 0.0f;
};

/// What a brush stroke does to the ground under it.
enum class sculpt_mode
{
    /// Pulls the surface up.
    raise,

    /// Pushes it down.
    lower,

    /// Averages each sample with its neighbours, wearing edges off.
    smooth,

    /// Pulls everything towards a given height, which is how a building gets
    /// somewhere level to stand.
    flatten
};

/**
 * @brief Rules for sprinkling props across the surface.
 *
 * Placements that fail the filters are retried rather than nudged, so a strict
 * filter thins the scatter out instead of bunching props against its edge.
 */
struct terrain_scatter
{
    int seed = 1337;

    /// How many props to aim for. Fewer are placed if the filters are tight.
    int count = 100;

    /// Steepest ground a prop will stand on, in degrees.
    float max_slope = 90.0f;

    /// Height band, in world space, the prop is allowed in.
    float min_height = -1e9f;
    float max_height = 1e9f;

    /// Radius around the terrain's centre left clear.
    float clear_radius = 0.0f;

    /// Strip along the outside edge left clear, in world units.
    float margin = 0.0f;

    /**
     * @brief Uniform scale, drawn from [x, y].
     *
     * Multiplied into whatever scale the prop already has rather than replacing
     * it, so a factory can shape a prop -- flatten a box into a slab, say -- and
     * still have scatter vary its size.
     */
    glm::vec2 scale_range = glm::vec2(1.0f, 1.0f);

    /// Random rotation about Y.
    bool random_yaw = true;

    /**
     * @brief Leans the prop with the ground instead of standing it upright.
     *
     * Exact for ground that falls away along Z, and an approximation once yaw
     * and a sideways lean are combined -- the object's euler angles are applied
     * X, then Y, then Z, which is the wrong order to compose a tilt on top of a
     * yaw. Close enough for rocks and bushes on gentle ground; stand anything
     * tall upright instead.
     */
    bool align_to_normal = false;

    /// Placement attempts per prop before that prop is given up on.
    int attempts_per_prop = 24;
};

/**
 * @brief A square patch of ground, centred on the object's position.
 *
 * Flat by default. set_noise() generates rolling ground, or set_height_function()
 * supplies your own surface; the mesh, its normals, height_at() and picking all
 * follow from that one function, so what you see and what you pick cannot
 * disagree.
 *
 *     auto ground = make_ref<terrain_3d>(256.0f, 128);
 *     ground->set_noise({ .frequency = 0.012f, .amplitude = 9.0f, .flat_radius = 20.0f });
 *     ground->set_layers({ { 0.0f, sand }, { 2.0f, grass }, { 7.0f, rock } });
 *     scene->add_child(ground);
 *
 * Decoration goes through decorate() and scatter(), which place props on the
 * surface and hand them to the scene the terrain belongs to. They are put in
 * the scene rather than parented to the terrain because the renderer draws the
 * scene's own render objects and does not walk further down the tree -- a prop
 * added with terrain->add_child() would never be drawn. Order does not matter:
 * decorating a terrain that is not in a scene yet holds the props until it is.
 */
class terrain_3d : public mesh_instance_3d
{
public:
    /**
     * @param size       world units along each edge
     * @param resolution tiles along each edge; vertices are not shared between
     *                   tiles, so the checker pattern has hard edges
     */
    terrain_3d(float size = 100.0f, int resolution = 50);
    virtual ~terrain_3d();

    /**
     * @brief Sets the surface height, sampled in the terrain's local XZ space,
     *        and rebuilds the mesh.
     *
     * Pass nullptr to go back to flat, which also makes picking exact rather
     * than iterative.
     */
    void set_height_function(std::function<float(float, float)> height);

    // ---- sculpting ------------------------------------------------------

    /// Height samples along each edge. One more than the tile resolution,
    /// since a row of tiles has a sample at each end.
    int samples() const;

    /**
     * @brief The editable heightfield, row-major from -half to +half.
     *
     * Empty until something sculpts or bakes. While it is empty the surface
     * comes from the height function instead, which is what noise sets.
     */
    const std::vector<float>& heightmap() const;

    /// Replaces it wholesale. Wrong-sized input is refused.
    bool set_heightmap(std::vector<float> heights);

    /// Whether the surface comes from the heightfield rather than a function
    /// -- which is to say, whether it has been shaped by hand.
    bool sculpted() const;

    /**
     * @brief Samples the current surface into the heightfield.
     *
     * Generation and sculpting meet here: noise lays the ground out, this
     * freezes it, and from then on a brush has something to push around.
     * Called for you the first time anything sculpts.
     */
    void bake_heightmap();

    /// Throws the heightfield away, going back to whatever the function says.
    void clear_heightmap();

    /**
     * @brief Applies one brush stroke, centred on a world position.
     *
     * The brush falls off towards its edge, so overlapping strokes build up
     * smoothly rather than in discs. @p strength is in world units per second
     * for raise and lower, and a rate of approach for smooth and flatten.
     *
     * @param level the height flatten pulls towards. Ignored by the others.
     * @return whether anything moved, so a caller can skip rebuilding.
     */
    bool sculpt(const glm::vec3& center, float radius, float strength,
                sculpt_mode mode, float delta_time, float level = 0.0f);

    /**
     * @brief Generates a fractal heightfield, replacing any height function.
     */
    void set_noise(const terrain_noise& noise);
    const terrain_noise& noise() const;

    /// Whether the current surface came from set_noise().
    bool generated() const;

    /// Alternating tile colours. Used only while no layers are set.
    void set_colors(const glm::vec3& first, const glm::vec3& second);

    /**
     * @brief Colours the ground by height, replacing the checkerboard.
     *
     * Heights are in the terrain's local space, so they line up with
     * terrain_noise::amplitude. The stops are sorted for you. Pass an empty
     * list to go back to the checkerboard.
     */
    void set_layers(std::vector<terrain_layer> layers);
    const std::vector<terrain_layer>& layers() const;

    /**
     * @brief Blends @p color in where the ground is steeper than @p angle.
     *
     * Exposed rock on hillsides, in other words. @p blend is the half-width of
     * the transition, in degrees.
     */
    void set_cliff(const glm::vec3& color, float angle = 38.0f, float blend = 12.0f);
    void clear_cliff();

    void rebuild();

    float size() const;
    int resolution() const;

    /// Height of the surface under a world-space XZ position.
    float height_at(float x, float z) const;

    /// Surface normal under a world-space XZ position.
    glm::vec3 normal_at(float x, float z) const;

    /// Angle between the surface and horizontal, in degrees.
    float slope_at(float x, float z) const;

    /// Whether a world-space XZ position is over the patch at all.
    bool contains(float x, float z) const;

    /// Copy of @p position moved vertically onto the surface.
    glm::vec3 place_on_surface(const glm::vec3& position) const;

    /**
     * @brief Sits @p prop on the ground at @p position and adds it to the scene.
     *
     * The prop's Y is taken from the surface; its X and Z are used as given.
     */
    void decorate(ref<render_object_3d> prop, const glm::vec3& position);

    /**
     * @brief Sprinkles instances of @p prop over the surface.
     * @return the props actually placed, which may be fewer than count.
     */
    std::vector<ref<render_object_3d>> scatter(ref<model> prop, const terrain_scatter& settings);

    /**
     * @brief As above, but you build each prop.
     *
     * @p factory is called with the index of the prop being placed and may
     * return nullptr to skip it, which is how you mix several kinds of prop
     * into one scatter.
     */
    std::vector<ref<render_object_3d>> scatter(const std::function<ref<render_object_3d>(int)>& factory,
                                               const terrain_scatter& settings);

    /// Everything decorate() and scatter() have placed.
    const std::vector<ref<render_object_3d>>& decorations() const;

    /// Removes them all from the scene.
    void clear_decorations();

    /**
     * @brief Where @p r first meets the ground.
     *
     * Flat terrain is solved directly. An uneven one is marched along the ray
     * until the ray drops below the surface, then bisected -- good enough for
     * picking a spot to aim a spell at, and it needs no acceleration structure.
     *
     * @return false if the ray never meets the patch.
     */
    bool raycast(const ray& r, glm::vec3& hit) const;

protected:
    /// Picks up props that were scattered before the terrain joined a scene.
    void set_scene(ref<render_object_3d> scene) override;

private:
    float m_size;
    int m_resolution;

    glm::vec3 m_first_color = glm::vec3(0.42f, 0.44f, 0.38f);
    glm::vec3 m_second_color = glm::vec3(0.33f, 0.35f, 0.30f);

    std::vector<terrain_layer> m_layers;

    bool m_has_cliff = false;
    glm::vec3 m_cliff_color = glm::vec3(0.40f, 0.38f, 0.35f);
    float m_cliff_angle = 38.0f;
    float m_cliff_blend = 12.0f;

    std::function<float(float, float)> m_height;

    /// (samples * samples) heights, row-major. Empty means "use m_height".
    std::vector<float> m_heights;

    /// Bilinear sample of the heightfield, in local space.
    float sample_heightmap(float x, float z) const;

    /**
     * @brief Whether the ground is anything other than the plane y = 0.
     *
     * Either a height function or a sculpted field counts. Asking about only
     * one of them is how a hand-made hill ends up drawn but not collided
     * with: the mesh comes from local_height(), which knows about both, while
     * anything testing m_height alone quietly takes the flat path.
     */
    bool has_surface() const;

    /**
     * @brief Builds the mesh as a shared-vertex grid.
     *
     * Which is what sculpting needs: a stroke then rebuilds (resolution + 1)^2
     * vertices rather than four per tile. At a resolution fine enough to sculpt
     * at, that is the difference between a brush that follows the mouse and one
     * that does not. The checkerboard still uses the old builder, since hard
     * tile edges are the one thing shared vertices cannot do.
     */
    ref<class mesh_3d> build_grid_mesh() const;

    /// Whether m_mesh is a grid, and so can be updated a patch at a time.
    bool m_mesh_is_grid = false;

    /**
     * @brief Re-does the vertices in a rectangle of samples, in place.
     *
     * What makes a brush usable: a stroke touches a few hundred vertices, and
     * rebuilding the mesh to report that costs more than the stroke did. The
     * rectangle is widened by one sample on each side, because a vertex's
     * normal is taken from the heights either side of it and those change too.
     */
    void refresh_region(int min_x, int max_x, int min_z, int max_z);

    terrain_noise m_noise;
    bool m_generated = false;

    std::vector<ref<render_object_3d>> m_decorations;

    /// Local height, before the object's own transform.
    float local_height(float x, float z) const;

    /// Local normal, from central differences over the height function.
    glm::vec3 local_normal(float x, float z) const;

    /// Colour of the surface at a local position, from the layers and the cliff.
    glm::vec3 surface_color(int tile_x, int tile_z, float local_y, const glm::vec3& normal) const;

    /// Adds a prop to the scene, or holds it until there is one.
    void attach(ref<render_object_3d> prop);

    ref<class mesh_3d> build_mesh() const;
};

} // namespace nle
