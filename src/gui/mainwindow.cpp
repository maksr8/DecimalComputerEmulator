#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CpuStats.h"
#include <QTableWidgetItem>
#include <QQuickWidget>
#include <QQmlContext>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QKeyEvent>
#include "../assembler/Assembler.h"
#include "../assembler/Disassembler.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow{ parent },
    _ui{ new Ui::MainWindow },
    _stats{ new CpuStats{ this } },
    _hasUnsavedChanges{ false },
    _hasUncompiledChanges{ false },
    _currentFilePath{ "" },
    _currentMode{ EmulationMode::STOPPED },
    _lastOutputSize{ 0 },
	_compiledBinary(1000, 0),
	_computer{}
{
    _ui->setupUi(this);

    _ui->splitter_main->setStretchFactor(0, 3);
    _ui->splitter_main->setStretchFactor(1, 3);
    _ui->splitter_main->setStretchFactor(2, 2);

    _ui->splitter_ram->setStretchFactor(0, 2);
    _ui->splitter_ram->setStretchFactor(1, 1);

    setupMemoryTables();
    _computer.loadProgram(_compiledBinary);

    _ui->pieChartWidget_instructionDistribution->rootContext()->setContextProperty("cpuData", _stats);
    QSurfaceFormat format;
	format.setSamples(8); // Anti-aliasing
    _ui->pieChartWidget_instructionDistribution->setFormat(format);
    _ui->pieChartWidget_instructionDistribution->setResizeMode(QQuickWidget::SizeRootObjectToView);
    _ui->pieChartWidget_instructionDistribution->setSource(QUrl(QStringLiteral("qrc:/qml/PieChart.qml")));

    _ui->lineEdit_minStepDuration->setValidator(new QIntValidator(0, 1000, this));

    setupConnections();

    _ui->horizontalSlider_minStepDuration->setValue(500);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::setupMemoryTables()
{
    auto setupTable = [this](QTableWidget* table)
        {
            table->setColumnCount(4);
            table->setHorizontalHeaderLabels({ " BP ", " Address ", " Data ", " Data Decoded " });
            table->setColumnWidth(0, 30);
            table->setColumnWidth(1, 60);
            table->setColumnWidth(2, 80);
            table->horizontalHeader()->setStretchLastSection(true);
            table->setEditTriggers(QAbstractItemView::NoEditTriggers);
            table->setSelectionBehavior(QAbstractItemView::SelectRows);
            table->verticalHeader()->setVisible(false);
            table->setRowCount(Config::MEMORY_SIZE);

            QColor memoryCellColor(45, 45, 48);

            for (int i = 0; i < Config::MEMORY_SIZE; ++i)
            {
                QTableWidgetItem* itemBP = new QTableWidgetItem("");
                itemBP->setTextAlignment(Qt::AlignCenter);

                QString addrStr = QString::number(i).rightJustified(3, '0');
                QTableWidgetItem* itemAddr = new QTableWidgetItem(addrStr);
                itemAddr->setTextAlignment(Qt::AlignCenter);

                QTableWidgetItem* itemData = new QTableWidgetItem("00000");
                itemData->setTextAlignment(Qt::AlignCenter);
                itemData->setBackground(QBrush(memoryCellColor));

                const Config::InstructionDef* def = Config::getInstructionDef(0);
                QTableWidgetItem* itemCode = new QTableWidgetItem((def->name).data());

                table->setItem(i, 0, itemBP);
                table->setItem(i, 1, itemAddr);
                table->setItem(i, 2, itemData);
                table->setItem(i, 3, itemCode);
            }
        };

    setupTable(_ui->tableMemory);
    setupTable(_ui->tableMemory_bottom);

    _ui->tableMemory->scrollToTop();
    _ui->tableMemory_bottom->scrollToBottom();
    _ui->tableMemory_bottom->horizontalHeader()->setVisible(false);
}

void MainWindow::setupConnections()
{
    connect(_ui->pushButton_open, &QPushButton::clicked, this, &MainWindow::onActionOpen);
    connect(_ui->pushButton_save, &QPushButton::clicked, this, &MainWindow::onActionSave);
    connect(_ui->pushButton_compile, &QPushButton::clicked, this, &MainWindow::onActionCompile);
    connect(_ui->pushButton_load, &QPushButton::clicked, this, &MainWindow::onActionLoad);

    connect(_ui->plainTextEdit_program, &QPlainTextEdit::textChanged, this, &MainWindow::onProgramTextChanged);

    connect(_ui->tableMemory, &QTableWidget::cellClicked, this, &MainWindow::onMemoryTableClicked);
    connect(_ui->tableMemory_bottom, &QTableWidget::cellClicked, this, &MainWindow::onMemoryTableClicked);

    connect(_ui->pushButton_run, &QPushButton::clicked, this, &MainWindow::onActionRun);
    connect(_ui->pushButton_debug, &QPushButton::clicked, this, &MainWindow::onActionDebug);
    connect(_ui->pushButton_step, &QPushButton::clicked, this, &MainWindow::onActionStep);
    connect(_ui->pushButton_continue, &QPushButton::clicked, this, &MainWindow::onActionDebug);
    connect(_ui->pushButton_stop, &QPushButton::clicked, this, &MainWindow::onActionStop);
    connect(_ui->pushButton_reset, &QPushButton::clicked, this, &MainWindow::onActionReset);
	connect(_ui->pushButton_enter, &QPushButton::clicked, this, &MainWindow::onActionEnter);

    connect(&_debugTimer, &QTimer::timeout, this, &MainWindow::onDebugTimerTick);

    connect(_ui->horizontalSlider_minStepDuration, &QSlider::valueChanged, this, &MainWindow::onMinStepDurationSliderChanged);
	connect(_ui->lineEdit_minStepDuration, &QLineEdit::textChanged, this, &MainWindow::onMinStepDurationChanged);
}

void MainWindow::onProgramTextChanged()
{
    if (!_hasUnsavedChanges)
    {
        _hasUnsavedChanges = true;
        if (_currentFilePath.isEmpty())
        {
            setWindowTitle("Decimal Computer Emulator - [Unsaved]*");
        }
        else
        {
            setWindowTitle(QString("Decimal Computer Emulator - %1*").arg(_currentFilePath));
        }
    }
    if (!_hasUncompiledChanges)
    {
        _hasUncompiledChanges = true;
        _ui->pushButton_load->setEnabled(false);
    }
}

bool MainWindow::promptSaveIfUnsaved()
{
    if (!_hasUnsavedChanges)
    {
        return true;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Unsaved Changes",
        "You have unsaved changes. Do you want to save them?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
    );

    if (reply == QMessageBox::Yes)
    {
        onActionSave();
        return !_hasUnsavedChanges;
    }

    if (reply == QMessageBox::Cancel)
    {
        return false;
    }

    return true;
}

void MainWindow::onActionOpen()
{
    if (!promptSaveIfUnsaved())
    {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, "Open Program", "", "Assembly Files (*.asm *.txt);;All Files (*)");
    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Could not open file.");
        return;
    }

    QTextStream in(&file);

    _ui->plainTextEdit_program->blockSignals(true);
    _ui->plainTextEdit_program->setPlainText(in.readAll());
    _ui->plainTextEdit_program->blockSignals(false);

    file.close();

    _currentFilePath = fileName;
    _hasUnsavedChanges = false;
    setWindowTitle(QString("Decimal Computer Emulator - %1").arg(_currentFilePath));
}

void MainWindow::onActionSave()
{
    QString fileName = _currentFilePath;

    if (fileName.isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(this, "Save Program", "", "Assembly Files (*.asm *.txt);;All Files (*)");
        if (fileName.isEmpty())
        {
            return;
        }
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Could not save file.");
        return;
    }

    QTextStream out(&file);
    out << _ui->plainTextEdit_program->toPlainText();
    file.close();

    _currentFilePath = fileName;
    _hasUnsavedChanges = false;
    setWindowTitle(QString("Decimal Computer Emulator - %1").arg(_currentFilePath));
}

void MainWindow::onActionCompile()
{
    Assembler assembler;
    std::string code = _ui->plainTextEdit_program->toPlainText().toStdString();

    AssemblerResult result = assembler.compile(code);

    if (!result.success)
    {
        QMessageBox::critical(this, "Compilation Error", QString::fromStdString(result.errorMessage));
        return;
    }

    _compiledBinary = result.machineCode;
    _currentLabels = result.reverseSymbolTable;

    _hasUncompiledChanges = false;
    _ui->pushButton_load->setEnabled(true);
    QMessageBox::information(this, "Success", "Program compiled successfully.");
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (promptSaveIfUnsaved())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::updateUI(bool fullUpdate)
{
    const CPU& cpu = _computer.getCPU();
    const Memory& memory = _computer.getMemory();

    _ui->lineEdit_acc->setText(QString::number(cpu.getAccumulator()));
    _ui->lineEdit_pc->setText(QString::number(cpu.getProgramCounter()).rightJustified(3, '0'));
    _ui->lineEdit_ir->setText(QString::number(cpu.getInstructionRegister()).rightJustified(5, '0'));
    _ui->lineEdit_sp->setText(QString::number(cpu.getStackPointer()).rightJustified(3, '0'));
    _ui->lineEdit_ixr->setText(QString::number(cpu.getIndexRegister()));
    _ui->lineEdit_steps->setText(QString::number(cpu.getCycles()));

    _ui->checkBox_overflow->setChecked(cpu.isOverflow());
    _ui->checkBox_halted->setChecked(cpu.isHalted());
    _ui->checkBox_waitingForInput->setChecked(cpu.isWaitingForInput());

    _ui->tableMemory->setUpdatesEnabled(false);
    _ui->tableMemory_bottom->setUpdatesEnabled(false);

    for (int i = 0; i < Config::MEMORY_SIZE; ++i)
    {
        int value = memory.read(i);
        QString valueStr;
        if (value < 0) {
            valueStr = "-" + QString::number(std::abs(value)).rightJustified(4, '0');
        }
        else {
            valueStr = QString::number(value).rightJustified(5, '0');
        }

        if (fullUpdate || _ui->tableMemory->item(i, 2)->text() != valueStr)
        {
            std::string decodedText = Disassembler::decode(value);
            if (_currentLabels.find(i) != _currentLabels.end())
            {
                decodedText = "[" + _currentLabels.at(i) + "] " + decodedText;
            }
            QString decodedQStr = QString::fromStdString(decodedText);

            _ui->tableMemory->item(i, 2)->setText(valueStr);
            _ui->tableMemory->item(i, 3)->setText(decodedQStr);

            _ui->tableMemory_bottom->item(i, 2)->setText(valueStr);
            _ui->tableMemory_bottom->item(i, 3)->setText(decodedQStr);
        }
    }

    _ui->tableMemory->setUpdatesEnabled(true);
    _ui->tableMemory_bottom->setUpdatesEnabled(true);

    _stats->updateStats(cpu.getStatALU(), cpu.getStatMemory(), cpu.getStatControl(), cpu.getStatIO());
    appendNewOutput();

    if (_currentMode != EmulationMode::RUNNING)
    {
        highlightCurrentInstruction();
    }
}

void MainWindow::highlightCurrentInstruction()
{
    int pc = _computer.getCPU().getProgramCounter();

    _ui->tableMemory->clearSelection();
    _ui->tableMemory_bottom->clearSelection();

    _ui->tableMemory->selectRow(pc);
    _ui->tableMemory_bottom->selectRow(pc);

    _ui->tableMemory->scrollToItem(_ui->tableMemory->item(pc, 0), QAbstractItemView::PositionAtCenter);
    _ui->tableMemory_bottom->scrollToItem(_ui->tableMemory_bottom->item(pc, 0), QAbstractItemView::PositionAtCenter);
}

void MainWindow::setUiInteractionEnabled(bool enabled)
{
    _ui->pushButton_compile->setEnabled(enabled);
    _ui->pushButton_load->setEnabled(enabled && !_hasUncompiledChanges);
    _ui->pushButton_run->setEnabled(enabled);
    _ui->pushButton_debug->setEnabled(enabled);
    _ui->pushButton_step->setEnabled(enabled);
    _ui->pushButton_reset->setEnabled(enabled);

    _ui->plainTextEdit_program->setReadOnly(!enabled);
    _ui->plainTextEdit_input->setReadOnly(!enabled);
}

// ret true on success, false on failure (invalid input)
bool MainWindow::parseInputTokens()
{
    std::queue<int> empty;
    std::swap(_inputTokens, empty);

    QString inputText = _ui->plainTextEdit_input->toPlainText().trimmed();
    if (inputText.isEmpty()) return true;

    QStringList strTokens = inputText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString& str : strTokens)
    {
        bool ok;
        int val = str.toInt(&ok);
        if (!ok || std::abs(val) >= Config::OVERFLOW_LIMIT)
        {
			QMessageBox::critical(this, "Input Error", "Invalid input token: '" + str + "'. Must be a valid integer within +-" + QString::number(Config::OVERFLOW_LIMIT));
            return false;
        }
        _inputTokens.push(val);
    }

    _ui->plainTextEdit_input->setReadOnly(true);
    return true;
}

void MainWindow::processRunChunk()
{
    if (_currentMode != EmulationMode::RUNNING) return;

    int instructionsExecuted = 0;
    constexpr int CHUNK_SIZE = 1000;

    while (!_computer.getCPU().isHalted() && !_computer.getCPU().isWaitingForInput())
    {
        try
        {
            _computer.step();
            instructionsExecuted++;

            if (instructionsExecuted >= CHUNK_SIZE)
            {
                appendNewOutput();
                QCoreApplication::processEvents();
                if (_currentMode != EmulationMode::RUNNING) return;
                instructionsExecuted = 0;
            }
        }
        catch (const std::exception& e)
        {
            onActionStop();
            QMessageBox::critical(this, "Runtime Error", e.what());
            return;
        }
    }

    appendNewOutput();

    if (_computer.getCPU().isWaitingForInput())
    {
        updateUI(true);
        _ui->plainTextEdit_input->setReadOnly(false);
        _ui->plainTextEdit_input->setFocus();
		_ui->pushButton_enter->setEnabled(true);
        return;
    }

    if (_computer.getCPU().isHalted())
    {
        onActionStop();
    }
}

void MainWindow::onDebugTimerTick()
{
    if (_currentMode != EmulationMode::DEBUGGING)
    {
        _debugTimer.stop();
        return;
    }

    if (_computer.getCPU().isHalted() || _computer.getCPU().isWaitingForInput())
    {
        _debugTimer.stop();
        if (_computer.getCPU().isWaitingForInput()) {
            _ui->plainTextEdit_input->setReadOnly(false);
            _ui->plainTextEdit_input->setFocus();
			_ui->pushButton_enter->setEnabled(true);
        }
        updateUI(true);
        if (_computer.getCPU().isHalted()) onActionStop();
        return;
    }

    try
    {
        _computer.step();
        updateUI(false);

        int nextPc = _computer.getCPU().getProgramCounter();
        if (_breakpoints.count(nextPc))
        {
            _debugTimer.stop();
            _currentMode = EmulationMode::STOPPED;
            setUiInteractionEnabled(true);
            QMessageBox::information(this, "Breakpoint", "Hit breakpoint at address " + QString::number(nextPc));
        }
    }
    catch (const std::exception& e)
    {
        onActionStop();
        QMessageBox::critical(this, "Runtime Error", e.what());
    }
}

void MainWindow::onActionRun()
{
    if (_compiledBinary.empty() || _computer.getCPU().isHalted()) return;

    if (_computer.getCPU().isWaitingForInput() && _inputTokens.empty())
    {
        return;
    }

    if (!_computer.getCPU().isWaitingForInput() && !parseInputTokens()) return;

    _currentMode = EmulationMode::RUNNING;
    setUiInteractionEnabled(false);
    _ui->pushButton_stop->setEnabled(true);

    processRunChunk();
}

void MainWindow::onActionDebug()
{
    if (_compiledBinary.empty() || _computer.getCPU().isHalted()) return;

    if (!_computer.getCPU().isWaitingForInput() && !parseInputTokens()) return;

    _currentMode = EmulationMode::DEBUGGING;
    setUiInteractionEnabled(false);
    _ui->pushButton_stop->setEnabled(true);
    _ui->pushButton_step->setEnabled(false);

    _debugTimer.start();
}

void MainWindow::onActionStep()
{
    if (_compiledBinary.empty() || _computer.getCPU().isHalted() || _computer.getCPU().isWaitingForInput()) return;

    try
    {
        _computer.step();
        updateUI(true);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Runtime Error", e.what());
    }
}

void MainWindow::onActionStop()
{
    _currentMode = EmulationMode::STOPPED;
    _debugTimer.stop();
    setUiInteractionEnabled(true);
    updateUI(true);
}

void MainWindow::onActionReset()
{
    onActionStop();

    _computer.reset();
    _computer.loadProgram(_compiledBinary);

    _lastOutputSize = 0;
    _ui->plainTextEdit_output->clear();

    std::queue<int> empty;
    std::swap(_inputTokens, empty);
    _ui->plainTextEdit_input->setReadOnly(false);

    updateUI(true);
}

void MainWindow::onActionEnter()
{
	if (!_computer.getCPU().isWaitingForInput()) return;

    if (parseInputTokens() && !_inputTokens.empty())
    {
        int val = _inputTokens.front();
        _inputTokens.pop();

        _computer.provideInput(val);

        if (_currentMode == EmulationMode::RUNNING) {
            processRunChunk();
        }
        else if (_currentMode == EmulationMode::DEBUGGING) {
            _debugTimer.start();
        }
		_ui->pushButton_enter->setEnabled(false);
    }
}

void MainWindow::onActionLoad()
{
    if (_compiledBinary.empty()) return;

    try
    {
        _computer.reset();
        _computer.loadProgram(_compiledBinary);

        _hasUncompiledChanges = false;
        _ui->pushButton_load->setEnabled(false);
        _lastOutputSize = 0;
        _ui->plainTextEdit_output->clear();

        updateUI(true);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Load Error", e.what());
    }
}

void MainWindow::onMemoryTableClicked(int row, int column)
{
    if (column == 0)
    {
        toggleBreakpoint(row);
    }
}

void MainWindow::toggleBreakpoint(int row)
{
    if (_breakpoints.count(row))
    {
        _breakpoints.erase(row);
        _ui->tableMemory->item(row, 0)->setText("");
        _ui->tableMemory->item(row, 0)->setBackground(Qt::transparent);
        _ui->tableMemory_bottom->item(row, 0)->setText("");
        _ui->tableMemory_bottom->item(row, 0)->setBackground(Qt::transparent);
    }
    else
    {
        _breakpoints.insert(row);
        _ui->tableMemory->item(row, 0)->setText("⬤");
        _ui->tableMemory->item(row, 0)->setForeground(Qt::red);
        _ui->tableMemory_bottom->item(row, 0)->setText("⬤");
        _ui->tableMemory_bottom->item(row, 0)->setForeground(Qt::red);
    }
}

void MainWindow::appendNewOutput()
{
    const auto& buffer = _computer.getCPU().getOutputBuffer();
    if (buffer.size() > _lastOutputSize)
    {
        for (size_t i = _lastOutputSize; i < buffer.size(); ++i)
        {
            _ui->plainTextEdit_output->appendPlainText(QString::number(buffer[i]));
        }
        _lastOutputSize = buffer.size();
    }
}

void MainWindow::onMinStepDurationChanged(const QString& value)
{
    int val = value.toInt();

    _ui->horizontalSlider_minStepDuration->blockSignals(true);
    _ui->horizontalSlider_minStepDuration->setValue(val);
    _ui->horizontalSlider_minStepDuration->blockSignals(false);

    _debugTimer.setInterval(val);
}

void MainWindow::onMinStepDurationSliderChanged(int value)
{
    _ui->lineEdit_minStepDuration->blockSignals(true);
    _ui->lineEdit_minStepDuration->setText(QString::number(value));
    _ui->lineEdit_minStepDuration->blockSignals(false);

    _debugTimer.setInterval(value);
}