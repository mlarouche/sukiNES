#pragma once

#include <QtWidgets/QDialog>

#include "ui_ppuvideodialog.h"

namespace sukiNES
{
	class PPU;
}

class PPUVideoDialog : public QDialog
{
	Q_OBJECT
public:
	PPUVideoDialog(QWidget* parent = nullptr);
	~PPUVideoDialog();

public slots:
	void updatePPUInfo(sukiNES::PPU* ppu);

private:
	void _updateMirroringType(sukiNES::PPU* ppu);
	void _updatePalette(sukiNES::PPU* ppu);
	void _updateNametable(sukiNES::PPU* ppu);

private:
	Ui::PPUVideoDialog _ui;

	QRgb _palette[64];
};