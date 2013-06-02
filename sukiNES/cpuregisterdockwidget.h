#pragma once

#include <QtWidgets/QDockWidget>

#include "ui_cpuregisterdockwidget.h"

namespace sukiNES
{
	class Cpu;
}

class QCloseEvent;

class CpuRegisterDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	CpuRegisterDockWidget(QWidget* parent = nullptr);
	~CpuRegisterDockWidget();

signals:
	void onClosed();

public slots:
	void cpuUpdated(sukiNES::Cpu* cpu);

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	Ui::CpuRegisterDockWidget _ui;
};