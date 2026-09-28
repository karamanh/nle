/**
 * @file retarget.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief Playing one rig's animations on another.
 * @version 0.1
 * @date 2026-09-28
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "animation.h"
#include "skeleton.h"

#include <vector>

namespace nle
{

/**
 * @brief Which bone of @p source each node of @p target is.
 *
 * Worked out from the shape of the two rigs rather than from their names,
 * because auto-riggers do not agree on names and do not always get them
 * right: one exporter's "Neck" is the first bone of another's spine. What
 * they do agree on is the shape of a person -- a hips with two legs going
 * down and a spine going up, a chest that branches into two arms and a
 * neck -- and that is what is matched.
 *
 * Chains of different lengths are matched from their roots, so a spine of
 * three bones against one of four leaves one bone unmatched rather than
 * every bone shifted by one.
 *
 * @return one entry per node of @p target: the index of the node of
 *         @p source it corresponds to, or -1 for a node that has none. All
 *         -1 when either rig is not recognisably a person.
 */
std::vector<int> match_humanoid_bones(const skeleton& source, const skeleton& target);

/**
 * @brief @p clip, as authored for @p source, re-expressed for @p target.
 *
 * Each matched bone is turned in the world by as much as its counterpart
 * turns away from its own bind pose, after first being aligned with the
 * direction its counterpart points at bind -- which is what lets an A-pose
 * drive a slightly different A-pose without every limb being off by the
 * difference. Bones keep their own lengths; only the hips move, scaled by
 * how tall the two rigs stand.
 *
 * Unmatched bones keep their rest pose relative to their parent, so a foot
 * the source rig never had simply follows its leg.
 *
 * @param bones as returned by match_humanoid_bones(source, target).
 * @return nullptr when nothing could be matched.
 */
ref<animation_clip> retarget_clip(const animation_clip& clip,
                                  const skeleton& source, const skeleton& target,
                                  const std::vector<int>& bones);

/**
 * @brief How a retargeted bone sits against its counterpart, in rotation.
 *
 * Once retargeted, @p node of @p target is always turned from its
 * counterpart in @p source by the same amount: its world rotation is the
 * source bone's times this. What hangs off a hand can be placed once, in
 * the source's hand, and carried onto every model that borrows its clips by
 * undoing this.
 *
 * Identity for a node with no counterpart.
 */
glm::quat retarget_offset(const skeleton& source, const skeleton& target,
                          const std::vector<int>& bones, int node);

} // namespace nle
