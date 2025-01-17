#include "rhpch.hpp"
#include "Player.hpp"
#include "Engine/Scene/Scene.hpp"
#include "Engine/GameObject/PlayerManager.hpp"
const char* PlayerObject::_obj_type_PlayerObject = "Player";

PlayerObject::PlayerObject() :
    m_controlType(eControlType::Tank)
{
    speed = 6;
    rotateSpeed = 360;
    rotateToMovement = true;
    m_relativeSource = eRelativeSource::Character;
}

void PlayerObject::Update(double dt) {
	if (this->activeState == true) {
		switch (m_controlType) {
			case eControlType::Normal: {
				MoveForward(Input::getAxisState("MoveForward"));
				MoveRight(Input::getAxisState("MoveRight"));
				Rotate(-Input::getAxisState("Rotate"));
			} break;
			case eControlType::Tank: {
				MoveForward(Input::getAxisState("MoveForward"));
				MoveRight(Input::getAxisState("StrafeRight"));
				Rotate(-Input::getAxisState("MoveRight"));
			} break;
		}
	}

    CharacterObject::Update(dt);

    // Reset character if they fall off the stage
    if (Position.y < -10) {
        Position = vec3(0, 0.1f, 0);
        Velocity = vec3(0, 0, 0);
    }
}

void PlayerObject::Create(jsonObj node) {
    CharacterObject::Create(node);

    //Position = vec3(0, 2, 0);
	Position = vec3(Position.x, Position.y + 2, Position.z);
    Velocity = vec3(0, 0, 0);
    m_hullOffset = vec3(0, .5, 0);
    m_collisionHullId = cWorld.CreateNewCapsule(Position + m_hullOffset, 1, 0.5f);
    //m_collisionHullId = cWorld.CreateNewCubeHull(resource, Position+vec3(0,1,0), .75, 2, .75);
    //cWorld.getHullFromID(m_collisionHullId)->rotation.toYawPitchRoll(.001, 0, 0);
}

void PlayerObject::InputEvent(s32 key, s32 action) {
    //if (key == GLFW_KEY_F && action == GLFW_PRESS) {
    //    CameraFollowPlayer = !CameraFollowPlayer;
    //}
	if (this->activeState == true) {
		if (key == GLFW_KEY_T && action == GLFW_PRESS) {
			switch (m_relativeSource) {
			case eRelativeSource::World: {
				m_relativeSource = eRelativeSource::Camera;
			} break;
			case eRelativeSource::Camera: {
				m_relativeSource = eRelativeSource::Character;
			} break;
			case eRelativeSource::Character: {
				m_relativeSource = eRelativeSource::World;
			} break;
			}
		}

		if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
			switch (m_controlType) {
			case eControlType::Normal: {
				m_controlType = eControlType::Tank;
				m_relativeSource = eRelativeSource::Character;
			} break;
			case eControlType::Tank: {
				m_controlType = eControlType::Normal;
			} break;
			}
		}

		if ((key == GLFW_KEY_SPACE && action == GLFW_PRESS) ||
			(key == GLFW_GAMEPAD_BUTTON_A && action == GLFW_PRESS)) {
			Jump(5.5);
		}

		if (key == GLFW_KEY_L && action == GLFW_PRESS) {
			switchActivePlayer();
		}
	}
}

const char* PlayerObject::GetControlType() {
    switch (m_controlType) {
        case eControlType::Normal: {
            return "Normal Controls";
        } break;
        case eControlType::Tank: {
            return "Tank Controls";
        } break;
    }
}

void PlayerObject::PostLoad() {
    //m_cameraID = GetCurrentScene()->getObjectIDByName("MainCamera"); // TODO: Should this REALLY be in here?
    //static_cast<Camera*>(GetCurrentScene()->getObjectByID(m_cameraID))->m_cameraMode = Camera::eCameraMode::ThirdPersonFollow;
	if (this->Name == "YaBoy") {
		this->activeState = true;
	}
    Input::registerGameObject(this);
}

const char* PlayerObject::ObjectTypeString() {
    return _obj_type_PlayerObject;
}

void PlayerObject::switchActivePlayer() {
	// TODO: Note that there should maybe be a redundant second check here to prevent multiple active players
	//static_cast<Camera*>(GetCurrentScene()->getObjectByID(m_cameraID))->changeFollowTarget(playerList[(thisID + 1) % size(playerList)]->getID());
	this->activeState = false;
	//auto it = std::find(GetCurrentScene()->objectsByType.Players.begin(), GetCurrentScene()->objectsByType.Players.end(), this);
	// TODO: Find the next element after this but not using getID() + 1
	//int index = std::distance(GetCurrentScene()->objectsByType.Players.begin(), it);
	//std::cout << GetCurrentScene()->objectsByType.Players[(index + 1) % GetCurrentScene()->objectsByType.Players.size()]->m_uid << " : Pre-Message ID\n";
}