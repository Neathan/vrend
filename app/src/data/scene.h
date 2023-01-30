#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "uuid.h"
#include "graphics/renderer.h"


class Scene;
class Entity {
public:
	Entity() : m_handle(), m_scene(nullptr) {}
	Entity(const entt::entity handle, Scene *scene)
		: m_handle(handle), m_scene(scene) {}

	template<typename T, typename... Args>
	T &addComponent(Args&&... args) {
		return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
	}

	template<typename T>
	T &getComponent() const {
		return m_scene->m_registry.get<T>(m_handle);
	}

	template<typename T>
	bool hasComponent() const {
		return m_scene->m_registry.all_of<T>(m_handle);
	}

	template<typename T>
	void removeComponent() {
		m_scene->m_registry.remove<T>(m_handle);
	}

	operator bool() const { return m_handle != entt::null; }
	operator uint32_t() const { return (uint32_t)m_handle; }

	bool operator==(const Entity &other) const {
		return m_handle == other.m_handle && m_scene == other.m_scene;
	}

	bool operator!=(const Entity &other) const {
		return !(*this == other);
	}

	glm::mat4 getWorldMatrix();
	glm::mat4 toLocalMatrix(glm::mat4 matrix);

private:
	friend Scene;

	entt::entity m_handle;
	Scene *m_scene;
};

class Scene {
public:
	Entity createEntity(const std::string &name = "Entity");
	void removeEntity(Entity entity);
	void removeEntity(UUID id);

	void render(Renderer& renderer);

	void destroy();
private:
	friend Entity;

	UUID m_uuid;
	entt::registry m_registry;
	std::unordered_map<UUID, Entity> m_entityMap;
};

struct IdentityComponent {
	UUID uuid = UUID();
	std::string name = "";
	std::vector<uint64_t> tags = {};
};

struct TransformComponent {
	glm::mat4 matrix = glm::mat4(1.0f);
	UUID parent = UUID::None();
};

