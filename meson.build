project(
  'cage',
  'c',
  version: '0.2.0',
  license: 'MIT',
  meson_version: '>=0.58.1',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'werror=true',
  ],
)

add_project_arguments(
  [
    '-DWLR_USE_UNSTABLE',
    '-Wundef',
    '-Wno-unused-parameter',
  ],
  language: 'c',
)

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DDEBUG', language: 'c')
endif

cc = meson.get_compiler('c')

is_freebsd = host_machine.system().startswith('freebsd')
if is_freebsd
  add_project_arguments(
    [
      '-Wno-format-extra-args',
      '-Wno-gnu-zero-variadic-macro-arguments',
    ],
    language: 'c',
  )
endif

wlroots = dependency('wlroots-0.19', fallback: ['wlroots', 'wlroots'])
wayland_protos = dependency('wayland-protocols', version: '>=1.14')
wlr_protos = dependency('wlr-protocols')
wayland_server = dependency('wayland-server')
wayland_client = dependency('wayland-client')
xkbcommon = dependency('xkbcommon')
math = cc.find_library('m')

wl_protocol_dir = wayland_protos.get_variable('pkgdatadir')
wlr_protocol_dir = wlr_protos.get_variable('pkgdatadir')
wayland_scanner = find_program('wayland-scanner')
wayland_scanner_server = generator(
  wayland_scanner,
  output: '@BASENAME@-protocol.h',
  arguments: ['server-header', '@INPUT@', '@OUTPUT@'],
)
wayland_scanner_client = generator(
  wayland_scanner,
  output: '@BASENAME@-client-protocol.h',
  arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)
wayland_scanner_code = generator(
  wayland_scanner,
  output: '@BASENAME@-protocol.c',
  arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

server_protocols = [
  [wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
]

server_protos_headers = []

foreach p : server_protocols
  xml = join_paths(p)
  server_protos_headers += wayland_scanner_server.process(xml)
endforeach

server_protos = declare_dependency(
  sources: server_protos_headers,
)

xdg_shell_protocol_xml = join_paths([wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'])
xdg_shell_protocol_code = wayland_scanner_code.process(xdg_shell_protocol_xml)
layer_shell_protocol_xml = join_paths([wlr_protocol_dir, 'unstable/wlr-layer-shell-unstable-v1.xml'])
layer_shell_protocol_client = wayland_scanner_client.process(layer_shell_protocol_xml)
layer_shell_protocol_code = wayland_scanner_code.process(layer_shell_protocol_xml)
layer_shell_protocol_dep = declare_dependency(
  sources: [layer_shell_protocol_client, layer_shell_protocol_code, xdg_shell_protocol_code],
)

have_xwayland = wlroots.get_variable(pkgconfig: 'have_xwayland', internal: 'have_xwayland') == 'true'

version = '@0@'.format(meson.project_version())
git = find_program('git', native: true, required: false)
if git.found()
  git_commit = run_command([git, 'rev-parse', '--short', 'HEAD'], check: false)
  git_branch = run_command([git, 'rev-parse', '--abbrev-ref', 'HEAD'], check: false)
  if git_commit.returncode() == 0 and git_branch.returncode() == 0
    version = '@0@-@1@ (branch \'@2@\')'.format(
      meson.project_version(),
      git_commit.stdout().strip(),
      git_branch.stdout().strip(),
    )
  endif
endif

conf_data = configuration_data()
conf_data.set10('CAGE_HAS_XWAYLAND', have_xwayland)
conf_data.set_quoted('CAGE_VERSION', version)

scdoc = dependency('scdoc', version: '>=1.9.2', native: true, required: get_option('man-pages'))
if scdoc.found()
  scdoc_prog = find_program(scdoc.get_variable('scdoc'), native: true)
  sh = find_program('sh', native: true)
  mandir = get_option('mandir')
  man_files = ['cage.1.scd']
  foreach filename : man_files
    topic = filename.split('.')[-3].split('/')[-1]
    section = filename.split('.')[-2]
    output = '@0@.@1@'.format(topic, section)

    custom_target(
      output,
      input: filename,
      output: output,
      command: [sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc_prog.full_path(), output)],
      install: true,
      install_dir: '@0@/man@1@'.format(mandir, section),
    )
  endforeach
endif

cage_sources = [
  'cage.c',
  'idle_inhibit_v1.c',
  'output.c',
  'seat.c',
  'view.c',
  'xdg_shell.c',
  'layer_shell_backend.c',
]

cage_headers = [
  configure_file(input: 'config.h.in', output: 'config.h', configuration: conf_data),
  'idle_inhibit_v1.h',
  'output.h',
  'seat.h',
  'server.h',
  'view.h',
  'xdg_shell.h',
  'layer_shell_backend.h',
]

if conf_data.get('CAGE_HAS_XWAYLAND', 0) == 1
  cage_sources += 'xwayland.c'
  cage_headers += 'xwayland.h'
endif

executable(
  meson.project_name(),
  cage_sources + cage_headers,
  dependencies: [
    server_protos,
    layer_shell_protocol_dep,
    wayland_server,
    wayland_client,
    wlroots,
    xkbcommon,
    math,
  ],
  install: true,
)

summary = ['', 'Cage @0@'.format(version), '', '    xwayland: @0@'.format(have_xwayland), '']
message('\n'.join(summary))