#pragma once

// Qt includes
#include <QtWidgets/QDialog>

#include "ui_ppudebuginfodialog.h"

namespace sukiNES
{
	class PPU;
}

class QCloseEvent;

class PPUDebugInfoDialog : public QDialog
{
public:
	PPUDebugInfoDialog(QWidget* parent = nullptr);
	~PPUDebugInfoDialog();

public slots:
	void updatePPUInfo(sukiNES::PPU* ppu);

private:
	Ui::PPUDebugInfoDialog _ui;
};
