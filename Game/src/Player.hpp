#pragma once

#include "Engine.hpp"

class PlayerController : public Engine::ScriptableBase {
public:
    PlayerController() {}

    virtual void OnCreate() override {
        transformComponent = &GetComponent<Engine::TransformComponent>();
        colliderComponent  = &GetComponent<Engine::ColliderComponent>();
        //auto followTarget = GetScene().FindByName("frog");

        LOG_ASSERT(transformComponent, "PlayerController could not find a transform component");
        LOG_ASSERT(colliderComponent,  "PlayerController could not find a transform component");

        // Pos, Yaw, Pitch, Roll, Forward, Right, Up
        auto[_pos, _yaw, _pitch, _roll, _forward, _right, _up] = math::Decompose(transformComponent->Transform);
        position = _pos;
        yaw = _yaw;
        pitch = _pitch;
        Forward = _forward;
        Right = _right;
        Up = _up;

        ///TEMP
        yaw = 0;
        ///TEMP

        hull_offset = math::vec3(0, .5, 0);

        LOG_INFO("Player controller created on GameObject {0}!", GetGameObjectID());
    }

    void ToggleControl() {
        BeingControlled = !BeingControlled;
        if (BeingControlled)
            LOG_INFO("PlayerController now in control");
        else
            LOG_INFO("PlayerController no longer in control");
    }

    virtual void OnUpdate(double ts) override {
        if (BeingControlled) {
            math::vec3 velocity(0,0,0);

            // handle translation
            if (Input::IsKeyPressed(KEY_CODE_Q)) {
                velocity -= Right * moveSpeed; // Strafe Left
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_E)) {
                velocity += Right * moveSpeed; // Strafe Right
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_W)) {
                velocity += Forward * moveSpeed; // Walk Forward
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_S)) {
                velocity -= Forward * moveSpeed; // Walk Backward
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_SPACE)) {
                velocity += Up * moveSpeed; // Float up
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_LEFT_CONTROL)) {
                velocity -= Up * moveSpeed; // Descend Down
                updateTransform = true;
            } 
            
            // handle rotation
            if (Input::IsKeyPressed(KEY_CODE_A)) {
                yaw += rotSpeed * ts; // Rotate Left
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_D)) {
                yaw -= rotSpeed * ts; // Rotate Right
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_R)) {
                pitch += rotSpeed * ts; // Pitch Up
                updateTransform = true;
            } if (Input::IsKeyPressed(KEY_CODE_F)) {
                pitch -= rotSpeed * ts; // Pitch Down
                updateTransform = true;
            }

            int iterations = 0;
            auto collisionHullID = colliderComponent->HullID;
            ShapecastResult_multi res;
            vec3 ghostPosition;
            float m_floorAngleLimit = 35;
            bool grounded = false;
            vec3 m_floorUp;

            // Perform collision logic
            if (collisionHullID > 0) {
                // it has collision info
                res = cWorld.Shapecast_multi(collisionHullID, velocity);

                if (res.numContacts) {
                    vec3 p_target = position + velocity * ts;
                    vec3 p = p_target;
                    vec3 p_start = position;
                    ghostPosition = position + (velocity*res.planes[0].TOI);

                    for (iterations = 0; iterations < 8; iterations++) {
                        float errorAcc = 0;
                        for (int n = 0; n < res.numContacts; n++) {
                            auto plane = &res.planes[n];

                            if (plane->TOI < ts) {
                                vec3 contactPoint = plane->contact_point;
                                vec3 contactNormal = plane->contact_normal;

                                vec3 p_t;
                                if (n > 0) {
                                    p_t = p_start + velocity * (plane->TOI - res.planes[n - 1].TOI);
                                }
                                else {
                                    p_t = p_start + velocity * plane->TOI;
                                }
                                vec3 body2contact = contactPoint - p_t;

                                vec3 sv = ((p + body2contact) - contactPoint);
                                float s = ((p + body2contact) - contactPoint).dot(contactNormal);
                                if (s < 0) {
                                    errorAcc -= s;

                                    p -= (s - 1.0e-3f)*contactNormal;

                                    if (acos(contactNormal.dot(vec3(0, 1, 0))) < (m_floorAngleLimit * d2r)) {
                                        grounded = true;
                                        velocity.y = 0;
                                        m_floorUp = plane->contact_normal;
                                    }
                                }
                            }
                        }
                        float eps = 1e-9;
                        if (errorAcc < eps) {
                            // We went through en entire iteration without moving 
                            // the object, we can stop iterating now.
                            break;
                        }
                        p_start = p;
                    }

                    position = p;
                }
                else {
                    position += velocity * ts;
                    ghostPosition = position;
                }

                CollisionHull* hull = cWorld.getHullFromID(collisionHullID);
                hull->position = position + hull_offset;

                // TODO: Expand the ShapeCast function to allow for rotation
                //hull->rotation.toYawPitchRoll(YawPitchRoll.x, 0, 0);
            }
            else {
                // Just use normal movement.
                position += velocity * ts;
                ghostPosition = position;
            }

            if (grounded) {
                velocity = vec3(0, 0, 0);
            }
            else {
                velocity = vec3(0, velocity.y, 0);
            }

            if (updateTransform) {
                auto& transform = transformComponent->Transform;

                position += velocity * ts;

                transform = mat4();
                transform.translate(position);
                transform *= math::createYawPitchRollMatrix(yaw, 0.0f, pitch);
                auto[f, r, u] = math::GetUnitVectors(transform);
                Forward = f;
                Right = r;
                Up = u;
                updateTransform = false;
            }
        }
    }

private:
    Engine::TransformComponent* transformComponent;
    Engine::ColliderComponent* colliderComponent;

    math::vec3 Forward, Right, Up;

    float moveSpeed = 4.0f;
    float rotSpeed = 360.0f;
    math::vec3 hull_offset;
    //bool gounded = false;
    math::vec3 position;
    float yaw, pitch;
    bool updateTransform = true;
    bool BeingControlled = true;
};