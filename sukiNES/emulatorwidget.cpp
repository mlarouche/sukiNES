#include "emulatorwidget.h"

// Qt includes
#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

#include "nespalette.h"

static const uint32 ScalingFactor = 3;
static const uint32 ScreenWidth = 256*ScalingFactor;
static const uint32 ScreenHeight = 240*ScalingFactor;

EmulatorWidget::EmulatorWidget(QWidget* parent)
: QWidget(parent)
, _screenBuffer(ScreenWidth, ScreenHeight, QImage::Format_RGB32)
{
	setFocusPolicy(Qt::StrongFocus);

	getNESPalette(_palette, sizeof(_palette) / sizeof(QRgb));

	for(uint32 whichController=0; whichController<2; ++whichController)
	{
		_buttonStatus[whichController].raw = 0;
	}

	QTimer::singleShot(0, this, SLOT(callRepaint()));
}

EmulatorWidget::~EmulatorWidget()
{
}

QSize EmulatorWidget::minimumSizeHint() const
{
	return QSize(ScreenWidth, ScreenHeight);
}

void EmulatorWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	
	painter.drawPixmap(0,0, _screenPixmap);
}

void EmulatorWidget::keyPressEvent(QKeyEvent* event)
{
	switch(event->key())
	{
		case Qt::Key_Z:
			_buttonStatus[0].B = true;
			break;
		case Qt::Key_X:
			_buttonStatus[0].A = true;
			break;
		case Qt::Key_Left:
			_buttonStatus[0].Left = true;
			break;
		case Qt::Key_Right:
			_buttonStatus[0].Right = true;
			break;
		case Qt::Key_Up:
			_buttonStatus[0].Up = true;
			break;
		case Qt::Key_Down:
			_buttonStatus[0].Down = true;
			break;
		case Qt::Key_Control:
			_buttonStatus[0].Start = true;
			break;
		case Qt::Key_Shift:
			_buttonStatus[0].Select = true;
			break;
		default:
			break;
	}
}

void EmulatorWidget::keyReleaseEvent(QKeyEvent* event)
{
	switch(event->key())
	{
		case Qt::Key_Z:
			_buttonStatus[0].B = false;
			break;
		case Qt::Key_X:
			_buttonStatus[0].A = false;
			break;
		case Qt::Key_Left:
			_buttonStatus[0].Left = false;
			break;
		case Qt::Key_Right:
			_buttonStatus[0].Right = false;
			break;
		case Qt::Key_Up:
			_buttonStatus[0].Up = false;
			break;
		case Qt::Key_Down:
			_buttonStatus[0].Down = false;
			break;
		case Qt::Key_Control:
			_buttonStatus[0].Start = false;
			break;
		case Qt::Key_Shift:
			_buttonStatus[0].Select = false;
			break;
		default:
			break;
	}
}

void EmulatorWidget::clearScreen()
{
	_screenBuffer.fill(Qt::black);
	callRepaint();
}

void EmulatorWidget::putPixel(sint32 x, sint32 y, byte paletteIndex)
{
	if (ScalingFactor == 1)
	{
		_screenBuffer.setPixel(x, y, _palette[paletteIndex]);
	}
	else
	{
		for(uint32 scaleY=0; scaleY<ScalingFactor; ++scaleY)
		{
			for (uint32 scaleX=0; scaleX<ScalingFactor; ++scaleX)
			{
				_screenBuffer.setPixel(x*ScalingFactor+scaleX, y*ScalingFactor+scaleY, _palette[paletteIndex]);
			}
		}
	}
}

void EmulatorWidget::onVBlank()
{
	QTimer::singleShot(0, this, SLOT(callRepaint()));
}

void EmulatorWidget::callRepaint()
{
	_screenPixmap = QPixmap::fromImage(_screenBuffer);
	repaint();
}

byte EmulatorWidget::inputStatus(byte controller) const
{
	return _buttonStatus[controller].raw;
}
