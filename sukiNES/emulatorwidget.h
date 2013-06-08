#pragma once

// Qt includes
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

// sukiNES includes
#include <ppuio.h>
#include <inputio.h>
#include <platform_support.h>

class QKeyEvent;
class QPaintEvent;

class EmulatorWidget : public QWidget, public sukiNES::PPUIO, public sukiNES::InputIO
{
	Q_OBJECT
public:
	EmulatorWidget(QWidget* parent = nullptr);
	virtual ~EmulatorWidget();

	virtual QSize minimumSizeHint() const;

	void clearScreen();

public:
	// PPUIO
	virtual void putPixel(sint32 x, sint32 y, byte paletteIndex) override;
	virtual void onVBlank() override;

	// InputIO
	virtual byte inputStatus(byte controller) const override;

public slots:
	void callRepaint();

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
	QImage _screenBuffer;
	QPixmap _screenPixmap;

	QRgb _palette[64];

	union
	{
		byte raw;
		RegBit<0> A;
		RegBit<1> B;
		RegBit<2> Select;
		RegBit<3> Start;
		RegBit<4> Up;
		RegBit<5> Down;
		RegBit<6> Left;
		RegBit<7> Right;
	} _buttonStatus[2];
};
