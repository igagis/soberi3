#include <QtCore/QDir>
#include <QtGui/QMouseEvent>
#include <QX11Info>

#include <tride/Vector3.hpp>

#include <ting/timer.hpp>

#include "GLWindow.hpp"
#include "Shader.hpp"

using namespace ting;
using namespace tride;


GLWindow::GLWindow() :
		oldMousePos(0, 0),
		needsRedraw(false)
{
	this->setAttribute(Qt::WA_PaintOnScreen);
	this->setAttribute(Qt::WA_NoSystemBackground);

	this->InitGL();

	//post resize event to init GL viewport and other stuff related to window size
	this->PostResizeEvent();
}



GLWindow::~GLWindow(){
//	TRACE(<< "GLWindow::~GLWindow(): invoked" << std::endl)
	this->DeinitGL();
}



//override
void GLWindow::mousePressEvent(QMouseEvent *event){
	this->HandleMousePressOrReleaseEvent(event, true);
}



//override
void GLWindow::mouseReleaseEvent(QMouseEvent *event){
	this->HandleMousePressOrReleaseEvent(event, false);
}



void GLWindow::HandleMousePressOrReleaseEvent(QMouseEvent* event, bool isPressed){
	ASSERT(event)

	//it happens that x or y can be negative when releasing the button outside the window
	//so, clamp x and y
	Vec2i cursorPos(event->x(), event->y());
	ting::util::ClampBottom(cursorPos.x, 0);
	ting::util::ClampTop(cursorPos.x, std::max(this->width() - 1, 0));
	ting::util::ClampBottom(cursorPos.y, 0);
	ting::util::ClampTop(cursorPos.y, std::max(this->height() - 1, 0));

	ASSERT(cursorPos.x >= 0 && cursorPos.y >= 0)
	ASSERT(cursorPos.y < this->height())
	ASSERT(cursorPos.x < this->width())
	Vec2f pos = this->WindowPosToWidgetPos(cursorPos);

	this->oldMousePos = pos;

	Widget::EMouseButton button;
	switch(event->button()){
		case Qt::LeftButton:
			button = Widget::LEFT;
			break;
		case Qt::RightButton:
			button = Widget::RIGHT;
			break;
		case Qt::MidButton:
			button = Widget::MIDDLE;
			break;
		default:
			ASSERT_INFO(false, "Button code = " << int(event->button()))
			break;
	}

	this->rootWidget.OnMouseClick(pos, button, isPressed);
}



//override
void GLWindow::mouseMoveEvent(QMouseEvent *event){
	Vec2f pos = this->WindowPosToWidgetPos(Vec2i(event->x(), event->y()));

	Vec2f dpos = pos - this->oldMousePos;

	this->rootWidget.OnMouseMove(this->oldMousePos, pos, dpos);

	this->oldMousePos = pos;
}



void GLWindow::RenderIfNecessary(){
	if(!this->needsRedraw)
		return;

	this->needsRedraw = false;

	//TODO: check if window is visible
	
	ASSERT(glGetError() == GL_NO_ERROR)

	glClearColor(0.0f, 0, 0.0f, 1.0f);
	ASSERT(glGetError() == GL_NO_ERROR)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ASSERT(glGetError() == GL_NO_ERROR)

	glFrontFace(GL_CCW);
	ASSERT(glGetError() == GL_NO_ERROR)
	glCullFace(GL_FRONT);
	ASSERT(glGetError() == GL_NO_ERROR)
	glEnable(GL_CULL_FACE);
//    glEnable(GL_DEPTH_TEST);

	ASSERT(glGetError() == GL_NO_ERROR)

	if(ting::Ref<Widget> w = this->rootWidget.GetWidget()){
		tride::Matr4f m;
		m.Identity();

		if(this->IsPortraitMode()){
			ASSERT(this->width() >= 0 && this->height() >= 0)
			m.Translate(tride::Vec2f(1, -1));
			ASSERT(this->width() != 0 && this->height() != 0)
			m.Scale(2 / float(this->width()), 2 / float(this->height()));
			m.Rotate(Vec3f(0, 0, math::DPi<float>() / 2));
		}else{//landscape mode
			ASSERT(this->width() > 0)
			m.Translate(tride::Vec2f(-1, -1));
			m.Scale(2 / float(this->width()), 2 / float(this->height()));
		}

		this->rootWidget.Render(m);
	}

	ASSERT(glGetError() == GL_NO_ERROR)

	this->SwapBuffers();
}



//override
void GLWindow::resizeEvent(QResizeEvent* event){
	//NOTE: When the resizeEvent() is called, the widget already has its new geometry.

//	TRACE(<< "GLWindow::resizeEvent(): invoked" << std::endl)
	ASSERT(glGetError() == GL_NO_ERROR)
	glViewport(0, 0, this->width(), this->height());
	ASSERT(glGetError() == GL_NO_ERROR)

//	TRACE(<< "GLWindow::resizeEvent(): viewport set" << std::endl)

	//update widget dimensions by setting the widget again
	this->SetWidget(this->rootWidget.GetWidget());

//	TRACE(<< "GLWindow::resizeEvent(): widget set" << std::endl)

	//redraw
	this->SetDirty();
}



//override
void GLWindow::paintEvent(QPaintEvent *event){
	this->needsRedraw = true;
	this->RenderIfNecessary();
}



/*
 @Function		TestEGLError
 @Input			pszLocation		location in the program where the error took
								place. ie: function name
 @Return		bool			true if no EGL error was detected
 @Description	Tests for an EGL error and prints it
*/
static bool TestEGLError(const char* pszLocation){
	/*
		eglGetError returns the last error that has happened using egl,
		not the status of the last called function. The user has to
		check after every single egl call or at least once every frame.
	*/
	EGLint iErr = eglGetError();
	if (iErr != EGL_SUCCESS){
		printf("%s failed (%d).\n", pszLocation, iErr);
		return false;
	}

	return true;
}



void GLWindow::InitGL(){
	/*
		Step 1 - Get the default display.
		EGL uses the concept of a "display" which in most environments
		corresponds to a single physical screen. Since we usually want
		to draw to the main screen or only have a single screen to begin
		with, we let EGL pick the default display.
		Querying other displays is platform specific.
	*/
	{
		Display *xdpy = QX11Info::display();
		if(!xdpy){
			TRACE(<< "main(): X11 display is not open" << std::endl)
			ASSERT(false)
		}
		this->eglDisplay = eglGetDisplay(EGLNativeDisplayType(xdpy));
	}

	/*
		Step 2 - Initialize EGL.
		EGL has to be initialized with the display obtained in the
		previous step. We cannot use other EGL functions except
		eglGetDisplay and eglGetError before eglInitialize has been
		called.
		If we're not interested in the EGL version number we can just
		pass NULL for the second and third parameters.
	*/
	EGLint iMajorVersion, iMinorVersion;
	if(!eglInitialize(this->eglDisplay, &iMajorVersion, &iMinorVersion)){
		printf("Error: eglInitialize() failed.\n");
//		CleanupAndExit (eglDisplay, x11Display, x11Window, x11Colormap);
		ASSERT(false)
	}

	/*
		Step 3 - Specify the required configuration attributes.
		An EGL "configuration" describes the pixel format and type of
		surfaces that can be used for drawing.
		For now we just want to use a 16 bit RGB surface that is a
		Window surface, i.e. it will be visible on screen. The list
		has to contain key/value pairs, terminated with EGL_NONE.
	 */
	EGLint pi32ConfigAttribs[5];
	pi32ConfigAttribs[0] = EGL_SURFACE_TYPE;
	pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
	pi32ConfigAttribs[2] = EGL_RENDERABLE_TYPE;
	pi32ConfigAttribs[3] = EGL_OPENGL_ES2_BIT;
	pi32ConfigAttribs[4] = EGL_NONE;

	EGLint pi32ContextAttribs[3];
	pi32ContextAttribs[0] = EGL_CONTEXT_CLIENT_VERSION;
	pi32ContextAttribs[1] = 2;
	pi32ContextAttribs[2] = EGL_NONE;

	/*
		Step 4 - Find a config that matches all requirements.
		eglChooseConfig provides a list of all available configurations
		that meet or exceed the requirements given as the second
		argument. In most cases we just want the first config that meets
		all criteria, so we can limit the number of configs returned to 1.
	*/
	EGLConfig eglConfig = 0;
	int iConfigs;
	if(!eglChooseConfig(this->eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1)){
		printf("Error: eglChooseConfig() failed.\n");
//		CleanupAndExit (eglDisplay, x11Display, x11Window, x11Colormap);
		ASSERT(false)
	}

	/*
		Step 5 - Create a surface to draw to.
		Use the config picked in the previous step and the native window
		handle when available to create a window surface. A window surface
		is one that will be visible on screen inside the native display (or
		fullscreen if there is no windowing system).
		Pixmaps and pbuffers are surfaces which only exist in off-screen
		memory.
	*/
	{
		EGLNativeWindowType windowDrawable = 0;
		windowDrawable = EGLNativeWindowType(this->winId());
		ASSERT(windowDrawable != 0)
		this->eglSurface = eglCreateWindowSurface(this->eglDisplay, eglConfig, windowDrawable, NULL);
	}

    if (!TestEGLError("eglCreateWindowSurface"))
	{
//		CleanupAndExit (eglDisplay, x11Display, x11Window, x11Colormap);
		ASSERT(false)
	}

	/*
		Step 6 - Create a context.
		EGL has to create a context for OpenGL ES. Our OpenGL ES resources
		like textures will only be valid inside this context
		(or shared contexts)
	*/
	this->eglContext = eglCreateContext(this->eglDisplay, eglConfig, NULL, pi32ContextAttribs);

	if (!TestEGLError("eglCreateContext"))
	{
//		CleanupAndExit (eglDisplay, x11Display, x11Window, x11Colormap);
		ASSERT(false)
	}

	/*
		Step 7 - Bind the context to the current thread and use our
		window surface for drawing and reading.
		Contexts are bound to a thread. This means you don't have to
		worry about other threads and processes interfering with your
		OpenGL ES application.
		We need to specify a surface that will be the target of all
		subsequent drawing operations, and one that will be the source
		of read operations. They can be the same surface.
	*/
	eglMakeCurrent(this->eglDisplay, this->eglSurface, this->eglSurface, this->eglContext);

	if (!TestEGLError("eglMakeCurrent"))
	{
//		CleanupAndExit (eglDisplay, x11Display, x11Window, x11Colormap);
		ASSERT(false)
	}
}



//NOTE: be sure to have this deinitialization called,
//      otherwise ~QApplication() destructor will hang.
void GLWindow::DeinitGL(){
	{
		DEBUG_CODE(EGLBoolean res =) eglMakeCurrent(this->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		ASSERT(res = EGL_TRUE)
	}

	{
		DEBUG_CODE(EGLBoolean res =) eglDestroyContext(this->eglDisplay, this->eglContext);
		ASSERT(res = EGL_TRUE)
	}

	{
		DEBUG_CODE(EGLBoolean res =) eglDestroySurface(this->eglDisplay, this->eglSurface);
		ASSERT(res = EGL_TRUE)
	}

	{
		DEBUG_CODE(EGLBoolean res =) eglTerminate(this->eglDisplay);
		ASSERT(res = EGL_TRUE)
	}
}



void GLWindow::SetWidget(ting::Ref<Widget> widget){
	if(widget.IsNotValid())
		return;

	widget->Move(tride::Vec2f(0, 0));

//	TRACE(<< "GLWindow::SetWidget(): resizing widget before setting as root widget" << std::endl)

	if(this->IsPortraitMode()){
		widget->Resize(tride::Vec2f(this->height(), this->width()));
	}else{
		widget->Resize(tride::Vec2f(this->width(), this->height()));
	}

//	TRACE(<< "GLWindow::SetWidget(): resizing done" << std::endl)

	this->rootWidget.SetWidget(widget);
//	TRACE(<< "GLWindow::SetWidget(): root widget set" << std::endl)
}



tride::Vec2f GLWindow::WindowPosToWidgetPos(const tride::Vec2i& p)const{
	if(this->IsPortraitMode()){
		return tride::Vec2f(this->height() - (p.y + 1), this->width() - (p.x + 1));
	}else{
		return tride::Vec2f(p.x, this->height() - (p.y + 1));
	}
}
