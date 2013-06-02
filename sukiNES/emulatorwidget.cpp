#include "emulatorwidget.h"

// Qt includes
#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

static const uint32 ScalingFactor = 3;
static const uint32 ScreenWidth = 256*ScalingFactor;
static const uint32 ScreenHeight = 240*ScalingFactor;

const unsigned char ntscpalette_pal[192] = {
    0x52, 0x52, 0x52, 0x01, 0x1A, 0x51, 0x0F, 0x0F, 0x65, 0x23, 0x06, 0x63, 0x36, 0x03, 0x4B, 0x40,
    0x04, 0x26, 0x3F, 0x09, 0x04, 0x32, 0x13, 0x00, 0x1F, 0x20, 0x00, 0x0B, 0x2A, 0x00, 0x00, 0x2F,
    0x00, 0x00, 0x2E, 0x0A, 0x00, 0x26, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xA0, 0xA0, 0xA0, 0x1E, 0x4A, 0x9D, 0x38, 0x37, 0xBC, 0x58, 0x28, 0xB8, 0x75, 0x21, 0x94, 0x84,
    0x23, 0x5C, 0x82, 0x2E, 0x24, 0x6F, 0x3F, 0x00, 0x51, 0x52, 0x00, 0x31, 0x63, 0x00, 0x1A, 0x6B,
    0x05, 0x0E, 0x69, 0x2E, 0x10, 0x5C, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFE, 0xFF, 0xFF, 0x69, 0x9E, 0xFC, 0x89, 0x87, 0xFF, 0xAE, 0x76, 0xFF, 0xCE, 0x6D, 0xF1, 0xE0,
    0x70, 0xB2, 0xDE, 0x7C, 0x70, 0xC8, 0x91, 0x3E, 0xA6, 0xA7, 0x25, 0x81, 0xBA, 0x28, 0x63, 0xC4,
    0x46, 0x54, 0xC1, 0x7D, 0x56, 0xB3, 0xC0, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFE, 0xFF, 0xFF, 0xBE, 0xD6, 0xFD, 0xCC, 0xCC, 0xFF, 0xDD, 0xC4, 0xFF, 0xEA, 0xC0, 0xF9, 0xF2,
    0xC1, 0xDF, 0xF1, 0xC7, 0xC2, 0xE8, 0xD0, 0xAA, 0xD9, 0xDA, 0x9D, 0xC9, 0xE2, 0x9E, 0xBC, 0xE6,
    0xAE, 0xB4, 0xE5, 0xC7, 0xB5, 0xDF, 0xE4, 0xA9, 0xA9, 0xA9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

EmulatorWidget::EmulatorWidget(QWidget* parent)
: QWidget(parent)
, _screenBuffer(ScreenWidth, ScreenHeight, QImage::Format_RGB32)
{
	setFocusPolicy(Qt::StrongFocus);

	for(uint32 i = 0; i<64; ++i)
	{
		byte r = ntscpalette_pal[(i * 3)];
		byte g = ntscpalette_pal[(i * 3) + 1];
		byte b = ntscpalette_pal[(i * 3) + 2];

		_palette[i] = qRgb(r, g, b);
	}

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
