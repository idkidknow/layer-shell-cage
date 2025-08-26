#ifndef CG_LAYER_SHELL_BACKEND_H
#define CG_LAYER_SHELL_BACKEND_H

#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <wayland-client-protocol.h>
#include <wayland-server-core.h>

struct host_outputs_element {
	struct cg_layer_shell_backend *backend;
	struct wl_output *output;
	const char *name;
	struct wl_list link;
};

struct cg_layer_shell_backend {
	struct wlr_backend *inner;
	struct wl_compositor *compositor;

	struct wl_list host_outputs;
	struct host_outputs_element *host_output;

	struct zwlr_layer_shell_v1 *layer_shell;
	uint32_t width;
	uint32_t height;
	struct wl_surface *surface;
	struct wlr_output *output;
};

struct cg_layer_shell_backend *layer_shell_backend_create(struct wl_event_loop *loop, struct wl_display *remote_display,
							  uint32_t layer, const char *output_name,
							  const char *namespace);

struct wlr_output *layer_shell_backend_output_create(struct cg_layer_shell_backend *backend);

#endif
