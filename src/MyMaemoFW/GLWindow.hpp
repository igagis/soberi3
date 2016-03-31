#pragma once

#include <ting/Singleton.hpp>
#include <ting/Ptr.hpp>
#include <ting/debug.hpp>

#include <tride/Vector3.hpp>

#include <QtGui/QMainWindow>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "BasicQMainWindow.hpp"
#include "Widget.hpp"
#include "Shader.hpp"
#include "Updater.hpp"



class GLWindow : public BasicQMainWindow, public ting::Singleton<GLWindow>{
	Q_OBJECT

	RootWidgetWrapper rootWidget;

public:
	void SetWidget(ting::Ref<Widget> widget);
	
	GLWindow();

	~GLWindow();

private:
	tride::Vec2f WindowPosToWidgetPos(const tride::Vec2i& p)const;
	
protected:
	//override
	virtual void paintEvent(QPaintEvent *event);

	//override
	virtual void mousePressEvent(QMouseEvent *event);

	//override
	virtual void mouseReleaseEvent(QMouseEvent *event);

private:
	void HandleMousePressOrReleaseEvent(QMouseEvent *event, bool isPressed);


	tride::Vec2f oldMousePos;
protected:
	//override
	virtual void mouseMoveEvent(QMouseEvent *event);

	//override
	virtual void resizeEvent(QResizeEvent *event);
private:
	bool needsRedraw;
	
public:
	inline void SetDirty(){
		this->needsRedraw = true;
	}

	void RenderIfNecessary();

private:
	EGLDisplay eglDisplay;
	EGLSurface eglSurface;
	EGLContext eglContext;

	void InitGL();

	void DeinitGL();

	inline void SwapBuffers(){
		eglSwapBuffers(this->eglDisplay, this->eglSurface);
	}
};
