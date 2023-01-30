#include "scene.h"

#include "data/model.h"


glm::mat4 Entity::getWorldMatrix() {
	TransformComponent &transformComponent = getComponent<TransformComponent>();
	glm::mat4 localMatrix = transformComponent.matrix;
	UUID parentID = transformComponent.parent;
	if (parentID.isValid()) {
		return m_scene->m_entityMap[parentID].getWorldMatrix() * localMatrix;
	}
	return localMatrix;
}

glm::mat4 Entity::toLocalMatrix(glm::mat4 matrix) {
	TransformComponent &transformComponent = getComponent<TransformComponent>();
	if (!transformComponent.parent.isValid()) {
		return matrix;
	}
	return glm::inverse(m_scene->m_entityMap[transformComponent.parent].getWorldMatrix()) * matrix;
}


Entity Scene::createEntity(const std::string &name) {
	Entity entity(m_registry.create(), this);
	entity.addComponent<TransformComponent>();
	IdentityComponent &identity = entity.addComponent<IdentityComponent>();
	identity.name = name;
	m_entityMap[identity.uuid] = entity;
	return entity;
}

void Scene::removeEntity(Entity entity) {
	removeEntity(entity.getComponent<IdentityComponent>().uuid);
}

void Scene::removeEntity(UUID id) {
	for (auto &[uuid, entity] : m_entityMap) {
		TransformComponent &transform = entity.getComponent<TransformComponent>();
		if (uuid != id && transform.parent == id) {
			transform.matrix = entity.getWorldMatrix();;
			transform.parent = UUID::None();
		}
	}
	m_registry.destroy(m_entityMap.at(id).m_handle);
	m_entityMap.erase(id);
}

void Scene::render(Renderer &renderer) {
	auto modelEntities = m_registry.view<TransformComponent, ModelComponent>();
	for (const auto &[entity, transformComponent, modelComponent] : modelEntities.each()) {
		renderer.addTransformCommand(transformComponent.matrix);
		renderer.addModelCommand(modelComponent.model.get());
	}
}

void Scene::destroy() {
	m_registry.clear();
}
