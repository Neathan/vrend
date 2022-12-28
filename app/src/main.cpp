#include "log.h"
#include "app.h"

int main() {
	LOG_SET_LEVEL(LOG_LEVEL_TRACE);
	LOG_DEBUG("Vulkan renderer started");

	App app{1920, 1080};
	app.init();
	app.start();

	return 0;
}
