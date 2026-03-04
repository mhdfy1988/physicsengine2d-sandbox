CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude
LDFLAGS = -lm
DEPFLAGS = -MMD -MP
SRCDIR = src
APPDIR_DWRITE = apps/sandbox_dwrite
TESTDIR = tests
BINDIR = bin
OBJDIR = obj
LIBDIR = lib

SOURCES = $(SRCDIR)/math.c $(SRCDIR)/shape.c $(SRCDIR)/body.c $(SRCDIR)/constraint.c $(SRCDIR)/collision.c $(SRCDIR)/physics.c $(SRCDIR)/physics_world.c $(SRCDIR)/physics_memory.c $(SRCDIR)/physics_ids.c $(SRCDIR)/physics_parallel.c $(SRCDIR)/physics_broadphase.c $(SRCDIR)/physics_contact_manager.c $(SRCDIR)/physics_raycast.c $(SRCDIR)/physics_step.c $(SRCDIR)/physics_collision_pipeline.c $(SRCDIR)/physics_resolve.c $(SRCDIR)/physics_solver.c $(SRCDIR)/physics_integrate.c
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)

SANDBOX_SRCS = $(APPDIR_DWRITE)/main.c $(APPDIR_DWRITE)/infrastructure/project_tree.c $(APPDIR_DWRITE)/infrastructure/snapshot_repo.c $(APPDIR_DWRITE)/infrastructure/ui_layout_repo.c $(APPDIR_DWRITE)/presentation/render/ui_icons.c $(APPDIR_DWRITE)/presentation/render/ui_text.c $(APPDIR_DWRITE)/presentation/render/ui_primitives.c $(APPDIR_DWRITE)/presentation/render/ui_widgets.c $(APPDIR_DWRITE)/domain/app_command.c $(APPDIR_DWRITE)/infrastructure/app_event_bus.c $(APPDIR_DWRITE)/application/app_controller.c $(APPDIR_DWRITE)/application/app_runtime.c $(APPDIR_DWRITE)/application/scene_catalog.c $(APPDIR_DWRITE)/application/scene_builder.c $(APPDIR_DWRITE)/application/history_service.c $(APPDIR_DWRITE)/application/runtime_param_service.c $(APPDIR_DWRITE)/presentation/input/input_mapping.c $(APPDIR_DWRITE)/presentation/input/menu_file_edit_actions.c $(APPDIR_DWRITE)/presentation/input/menu_view_physics_window_actions.c $(APPDIR_DWRITE)/presentation/input/menu_help_actions.c $(APPDIR_DWRITE)/presentation/input/menu_model.c
SANDBOX_ICON_RC = $(APPDIR_DWRITE)/app_icon.rc
SANDBOX_ICON_ICO = assets/icons/physics_sandbox.ico
SANDBOX_ICON_OBJ = $(OBJDIR)/app_icon.res.o
TEST_SRC = $(TESTDIR)/regression_tests.c

CORE_LIB = $(LIBDIR)/libphysics2d.a
SANDBOX_EXECUTABLE = $(BINDIR)/physics_sandbox
TEST_EXECUTABLE = $(BINDIR)/physics_tests

WIN_DWRITE_LIBS = -ld2d1 -ldwrite -lwindowscodecs -lole32 -luuid -lshcore -lgdi32 -luser32
WIN_GUI_FLAGS = -mwindows

all: sandbox

$(BINDIR):
	if not exist $(BINDIR) mkdir $(BINDIR)

$(OBJDIR):
	if not exist $(OBJDIR) mkdir $(OBJDIR)

$(LIBDIR):
	if not exist $(LIBDIR) mkdir $(LIBDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(CORE_LIB): $(OBJECTS) | $(LIBDIR)
	$(AR) rcs $@ $^

$(SANDBOX_ICON_OBJ): $(SANDBOX_ICON_RC) $(SANDBOX_ICON_ICO) | $(OBJDIR)
	windres $< -O coff -o $@

$(SANDBOX_EXECUTABLE): $(CORE_LIB) $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) $(CORE_LIB) -o $@ $(LDFLAGS) $(WIN_DWRITE_LIBS) $(WIN_GUI_FLAGS)

$(TEST_EXECUTABLE): $(CORE_LIB) $(TEST_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) $(TEST_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

core: $(CORE_LIB)

sandbox: $(SANDBOX_EXECUTABLE)

run: $(SANDBOX_EXECUTABLE)
	./$(SANDBOX_EXECUTABLE)

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

check-core:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_core_change.ps1

check-arch:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_arch_deps.ps1

check-api:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_api_surface.ps1

clean:
	if exist $(BINDIR) rmdir /S /Q $(BINDIR)
	if exist $(OBJDIR) rmdir /S /Q $(OBJDIR)
	if exist $(LIBDIR) rmdir /S /Q $(LIBDIR)

-include $(DEPS)

.PHONY: all core sandbox run test check-core check-arch check-api clean
