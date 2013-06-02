#include "cpuregisterdockwidget.h"

// sukiNES includes
#include <cpu.h>

CpuRegisterDockWidget::CpuRegisterDockWidget(QWidget* parent)
: QDockWidget(tr("CPU Registers"), parent)
{
	QWidget* theWidget = new QWidget(this);
	_ui.setupUi(theWidget);

	setWidget(theWidget);
}

CpuRegisterDockWidget::~CpuRegisterDockWidget()
{
}

void CpuRegisterDockWidget::closeEvent(QCloseEvent* event)
{
	emit onClosed();
	QDockWidget::closeEvent(event);
}

void CpuRegisterDockWidget::cpuUpdated(sukiNES::Cpu* cpu)
{
	auto cpuRegisters = cpu->getRegisters();

	_ui.lineA->setText( QString::number(cpuRegisters.A, 16).toUpper() );
	_ui.lineX->setText( QString::number(cpuRegisters.X, 16).toUpper() );
	_ui.lineY->setText( QString::number(cpuRegisters.Y, 16).toUpper() );
	_ui.linePC->setText( QString::number(cpuRegisters.ProgramCounter, 16).toUpper() );
	_ui.lineSP->setText( QString::number(cpuRegisters.StackPointer, 16).toUpper() );

	_ui.checkCarry->setChecked( (unsigned)cpuRegisters.ProcessorStatus.Carry );
	_ui.checkZero->setChecked( (unsigned)cpuRegisters.ProcessorStatus.Zero );
	_ui.checkInterruptDisabled->setChecked( (unsigned)cpuRegisters.ProcessorStatus.InterruptDisabled );
	_ui.checkDecimal->setChecked( (unsigned)cpuRegisters.ProcessorStatus.Decimal );
	_ui.checkBreak->setChecked( (unsigned)cpuRegisters.ProcessorStatus.Break );
	_ui.checkUnused->setChecked( (unsigned)cpuRegisters.ProcessorStatus.Unused );
	_ui.checkOverflow->setChecked( (unsigned)cpuRegisters.ProcessorStatus.Overflow );
	_ui.checkNegative->setChecked( (unsigned)cpuRegisters.ProcessorStatus.Negative );
}
