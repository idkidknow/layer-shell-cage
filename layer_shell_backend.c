#include "layer_shell_backend.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlr/util/log.h"
#include <stdlib.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wlr/backend/wayland.h>

static void
handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	struct cg_layer_shell_backend *backend = data;
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		backend->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		backend->layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 4);
	}
}

static void
handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

static void
handle_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface, uint32_t serial, uint32_t width,
			       uint32_t height)
{
	struct cg_layer_shell_backend *backend = data;
	backend->width = width;
	backend->height = height;
	zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_custom_mode(&state, width, height, 0);
	wlr_output_commit_state(backend->output, &state);
	wlr_output_state_finish(&state);

	wlr_log(WLR_INFO, "width: %d height: %d", width, height);
}

static void
handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1)
{
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = handle_layer_surface_configure,
	.closed = handle_layer_surface_closed,
};

struct cg_layer_shell_backend *
layer_shell_backend_create(struct wl_event_loop *loop, struct wl_display *remote_display)
{
	struct cg_layer_shell_backend *backend = calloc(1, sizeof(*backend));
	struct wlr_backend *inner_backend = wlr_wl_backend_create(loop, remote_display);
	backend->inner = inner_backend;
	struct wl_registry *registry = wl_display_get_registry(remote_display);
	wl_registry_add_listener(registry, &registry_listener, backend);
	wl_display_roundtrip(remote_display); // get globals
	struct wl_surface *surface = wl_compositor_create_surface(backend->compositor);
	backend->surface = surface;
	struct zwlr_layer_surface_v1 *layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		backend->layer_shell, surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "cage_layer");
	zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, backend);
	zwlr_layer_surface_v1_set_anchor(
		layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
				       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);
	zwlr_layer_surface_v1_set_keyboard_interactivity(layer_surface,
							 ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);
	return backend;
}

struct wlr_output *
layer_shell_backend_output_create(struct cg_layer_shell_backend *backend)
{
	struct wlr_output *output = wlr_wl_output_create_from_surface(backend->inner, backend->surface);
	backend->output = output;
	return output;
}
