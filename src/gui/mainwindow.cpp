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

constexpr int RUN_CHUNK_SIZE{ 5000 };
constexpr const char* WINDOW_TITLE_BASE{ "Decimal Computer Emulator" };

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow{ parent },
    _ui{ new Ui::MainWindow },
    _stats{ new CpuStats{ this } },
    _compiledBinary(Config::MEMORY_SIZE, 0)
{
    _ui->setupUi(this);

    setWindowTitle(WINDOW_TITLE_BASE);

    _ui->splitter_main->setStretchFactor(0, 3);
    _ui->splitter_main->setStretchFactor(1, 3);
    _ui->splitter_main->setStretchFactor(2, 2);

    _ui->splitter_ram->setStretchFactor(0, 2);
    _ui->splitter_ram->setStretchFactor(1, 1);

    setupMemoryTables();

    _ui->pieChartWidget_instructionDistribution->rootContext()->setContextProperty("cpuData", _stats);
    QSurfaceFormat format;
    format.setSamples(8); // Anti-aliasing
    _ui->pieChartWidget_instructionDistribution->setFormat(format);
    _ui->pieChartWidget_instructionDistribution->setResizeMode(QQuickWidget::SizeRootObjectToView);
    _ui->pieChartWidget_instructionDistribution->setSource(QUrl(QStringLiteral("qrc:/qml/PieChart.qml")));

    _ui->lineEdit_minStepDuration->setValidator(new QIntValidator(0, 1000, this));

    setupConnections();

    _ui->horizontalSlider_minStepDuration->setValue(500);

    applyStateToUI();
}

MainWindow::~MainWindow()
{
    delete _ui;
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

void MainWindow::setUnsavedChanges(bool value)
{
    if (_hasUnsavedChanges == value) return;

    _hasUnsavedChanges = value;

    if (_hasUnsavedChanges)
    {
        if (_currentFilePath.isEmpty())
        {
            setWindowTitle(QString("%1 - %2 *").arg(WINDOW_TITLE_BASE).arg("[Unsaved]"));
        }
        else
        {
            setWindowTitle(QString("%1 - %2 *").arg(WINDOW_TITLE_BASE).arg(_currentFilePath));
        }
    }

    applyStateToUI();
}

void MainWindow::setBinaryUpToDate(bool value)
{
    if (_isBinaryUpToDate == value) return;
    _isBinaryUpToDate = value;
    applyStateToUI();
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

                QTableWidgetItem* itemData = new QTableWidgetItem(" 00000");
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
    connect(_ui->pushButton_continue, &QPushButton::clicked, this, &MainWindow::onActionContinue);
    connect(_ui->pushButton_stop, &QPushButton::clicked, this, &MainWindow::onActionStop);
    connect(_ui->pushButton_reset, &QPushButton::clicked, this, &MainWindow::onActionReset);
    connect(_ui->pushButton_enter, &QPushButton::clicked, this, &MainWindow::onActionEnter);

    connect(&_debugTimer, &QTimer::timeout, this, &MainWindow::onDebugTimerTick);

    connect(_ui->horizontalSlider_minStepDuration, &QSlider::valueChanged, this, &MainWindow::onMinStepDurationSliderChanged);
    connect(_ui->lineEdit_minStepDuration, &QLineEdit::textChanged, this, &MainWindow::onMinStepDurationChanged);
}

bool MainWindow::isProgramMutableState() const
{
    return _currentState == AppState::EDITING ||
        _currentState == AppState::IDLE ||
        _currentState == AppState::RUN_FINISHED;
}

void MainWindow::onProgramTextChanged()
{
    if (!isProgramMutableState())
    {
        assert(false && "Program text change triggered in invalid state");
        return;
    }
    setUnsavedChanges(true);
    setBinaryUpToDate(false);
    changeState(AppState::EDITING);
}

void MainWindow::onActionOpen()
{
    if (!isProgramMutableState())
    {
        assert(false && "Open action triggered in invalid state");
        return;
    }
    if (!promptSaveIfUnsaved())
    {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, "Open Program", "",
        "Decimal Assembly (*.dasm);;Standard Assembly (*.asm);;Text Files (*.txt);;All Files (*)");
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

    setUnsavedChanges(false);
    setWindowTitle(QString("%1 - %2").arg(WINDOW_TITLE_BASE).arg(_currentFilePath));
    setBinaryUpToDate(false);
    changeState(AppState::EDITING);
}

void MainWindow::onActionSave()
{
    if (!isProgramMutableState())
    {
        assert(false && "Save action triggered in invalid state");
        return;
    }

    doSave();
}

void MainWindow::doSave()
{
    QString fileName;

    fileName = QFileDialog::getSaveFileName(this, "Save Program", "",
        "Decimal Assembly(*.dasm);; Standard Assembly(*.asm);; Text Files(*.txt);; All Files(*)");
    if (fileName.isEmpty())
    {
        return;
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
    setUnsavedChanges(false);
    setWindowTitle(QString("%1 - %2").arg(WINDOW_TITLE_BASE).arg(_currentFilePath));
}

void MainWindow::onActionCompile()
{
    if (_currentState != AppState::EDITING)
    {
        assert(false && "Compile action triggered in invalid state");
        return;
    }
    Assembler assembler;
    std::string program = _ui->plainTextEdit_program->toPlainText().toStdString();

    AssemblerResult result = assembler.compile(program);

    if (!result.success)
    {
        setBinaryUpToDate(false);
        QMessageBox::critical(this, "Compilation Error", QString::fromStdString(result.errorMessage));
        return;
    }

    _compiledBinary = result.machineCode;
    _currentLabels = result.reverseSymbolTable;

    setBinaryUpToDate(true);
}

void MainWindow::onActionLoad()
{
    if (_currentState != AppState::EDITING)
    {
        assert(false && "Load action triggered in invalid state");
        return;
    }

    try
    {
        _computer.reset();
        _computer.loadProgram(_compiledBinary);

        _ui->pushButton_load->setEnabled(false);
        _lastOutputSize = 0;
        _ui->plainTextEdit_output->clear();
        clearBreakpoints();
        std::queue<int> empty;
        std::swap(_inputTokens, empty);

        updateFullMemoryTable();
        updateRegistersFlagsStats();
        changeState(AppState::IDLE);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Load Error", e.what());
    }
}

// ret true if we should proceed with the action which
// may result in losing unsaved changes
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
        doSave();
        return !_hasUnsavedChanges;
    }

    if (reply == QMessageBox::Cancel)
    {
        return false;
    }

    return true; // lose changes
}

void MainWindow::updateFullMemoryTable()
{
    _ui->tableMemory->setUpdatesEnabled(false);
    _ui->tableMemory_bottom->setUpdatesEnabled(false);

    for (int i = 0; i < Config::MEMORY_SIZE; ++i)
    {
        updateSingleMemoryRow(i);
    }

    _ui->tableMemory->setUpdatesEnabled(true);
    _ui->tableMemory_bottom->setUpdatesEnabled(true);
}

void MainWindow::clearBreakpoints()
{
    for (int row : _breakpoints)
    {
        _ui->tableMemory->item(row, 0)->setText("");
        _ui->tableMemory->item(row, 0)->setBackground(Qt::transparent);
        _ui->tableMemory_bottom->item(row, 0)->setText("");
        _ui->tableMemory_bottom->item(row, 0)->setBackground(Qt::transparent);
    }
    _breakpoints.clear();
}

void MainWindow::updateRegistersFlagsStats()
{
    const CPU& cpu = _computer.getCPU();
    _ui->lineEdit_acc->setText(QString::number(cpu.getAccumulator()));
    _ui->lineEdit_pc->setText(QString::number(cpu.getProgramCounter()).rightJustified(3, '0'));
    _ui->lineEdit_ir->setText(QString::number(cpu.getInstructionRegister()).rightJustified(5, '0'));
    _ui->lineEdit_sp->setText(QString::number(cpu.getStackPointer()).rightJustified(3, '0'));
    _ui->lineEdit_ixr->setText(QString::number(cpu.getIndexRegister()));

    _ui->checkBox_overflow->setChecked(cpu.isOverflow());
    _ui->checkBox_halted->setChecked(cpu.isHalted());
    _ui->checkBox_waitingForInput->setChecked(cpu.isWaitingForInput());

    _ui->lineEdit_steps->setText(QString::number(cpu.getCycles()));
    _stats->updateStats(cpu.getStatALU(), cpu.getStatMemory(), cpu.getStatControl(), cpu.getStatIO());
}

void MainWindow::updateSingleMemoryRow(int address)
{
    if (address < 0 || address >= Config::MEMORY_SIZE) return;

    int value = _computer.getMemory().read(address);

    QString valueStr = QString("%1%2")
        .arg(value < 0 ? "-" : " ")
        .arg(std::abs(value), 5, 10, QChar('0'));

    QString decodedQStr = QString::fromStdString(Disassembler::decode(value));

    if (_currentLabels.find(address) != _currentLabels.end())
    {
        decodedQStr = QString("[%1] %2")
            .arg(QString::fromStdString(_currentLabels.at(address)), decodedQStr);
    }

    _ui->tableMemory->item(address, 2)->setText(valueStr);
    _ui->tableMemory->item(address, 3)->setText(decodedQStr);

    _ui->tableMemory_bottom->item(address, 2)->setText(valueStr);
    _ui->tableMemory_bottom->item(address, 3)->setText(decodedQStr);
}

void MainWindow::highlightTableRowsCurrent()
{
    int pc = _computer.getCPU().getProgramCounter();
    int sp = _computer.getCPU().getStackPointer();

    if (_computer.getCPU().isWaitingForInput() && pc > 0) {
        pc = pc - 1;
    }

    _ui->tableMemory->clearSelection();
    _ui->tableMemory_bottom->clearSelection();

    _ui->tableMemory->selectRow(pc);
    _ui->tableMemory->scrollToItem(_ui->tableMemory->item(pc, 0), QAbstractItemView::PositionAtCenter);

    _ui->tableMemory_bottom->selectRow(sp);
    _ui->tableMemory_bottom->scrollToItem(_ui->tableMemory_bottom->item(sp, 0), QAbstractItemView::PositionAtCenter);
}

void MainWindow::changeState(AppState newState)
{
    if (_currentState == newState) return;

    if (newState == AppState::WAITING_INPUT) {
        _stateBeforeInput = _currentState;
        _ui->plainTextEdit_input->setFocus();
    }

    _currentState = newState;
    applyStateToUI();
}

void MainWindow::applyStateToUI()
{
    _ui->pushButton_open->setEnabled(false);
    _ui->pushButton_save->setEnabled(false);
    _ui->pushButton_compile->setEnabled(false);
    _ui->pushButton_load->setEnabled(false);

    _ui->pushButton_run->setEnabled(false);
    _ui->pushButton_debug->setEnabled(false);
    _ui->pushButton_enter->setEnabled(false);

    _ui->pushButton_step->setEnabled(false);
    _ui->pushButton_continue->setEnabled(false);
    _ui->pushButton_stop->setEnabled(false);
    _ui->pushButton_reset->setEnabled(false);
    _ui->horizontalSlider_minStepDuration->setEnabled(false);

    _ui->plainTextEdit_program->setReadOnly(true);
    _ui->plainTextEdit_input->setReadOnly(true);
    _ui->lineEdit_minStepDuration->setReadOnly(true);

    switch (_currentState)
    {
    case AppState::EDITING:
        _ui->plainTextEdit_program->setReadOnly(false);
        _ui->pushButton_compile->setEnabled(!_isBinaryUpToDate);
        _ui->pushButton_open->setEnabled(true);
        _ui->pushButton_save->setEnabled(true);
        _ui->pushButton_load->setEnabled(_isBinaryUpToDate);
        break;

    case AppState::IDLE:
        _ui->plainTextEdit_program->setReadOnly(false);
        _ui->pushButton_open->setEnabled(true);
        _ui->pushButton_save->setEnabled(true);
        _ui->pushButton_run->setEnabled(true);
        _ui->pushButton_debug->setEnabled(true);
        _ui->pushButton_step->setEnabled(true);
        _ui->plainTextEdit_input->setReadOnly(false);
        _ui->lineEdit_minStepDuration->setReadOnly(false);
        _ui->horizontalSlider_minStepDuration->setEnabled(true);
        break;

    case AppState::RUNNING:
        _ui->pushButton_stop->setEnabled(true);
        _ui->pushButton_reset->setEnabled(true);
        break;
    case AppState::DEBUG_RUNNING:
        _ui->pushButton_stop->setEnabled(true);
        _ui->pushButton_reset->setEnabled(true);
        _ui->lineEdit_minStepDuration->setReadOnly(false);
        _ui->horizontalSlider_minStepDuration->setEnabled(true);
        break;

    case AppState::DEBUG_PAUSED:
        _ui->pushButton_run->setEnabled(true);
        _ui->pushButton_continue->setEnabled(true);
        _ui->pushButton_step->setEnabled(true);
        _ui->pushButton_stop->setEnabled(true);
        _ui->pushButton_reset->setEnabled(true);
        _ui->lineEdit_minStepDuration->setReadOnly(false);
        _ui->horizontalSlider_minStepDuration->setEnabled(true);
        break;

    case AppState::WAITING_INPUT:
        _ui->plainTextEdit_input->setReadOnly(false);
        _ui->pushButton_enter->setEnabled(true);
        _ui->pushButton_stop->setEnabled(true);
        _ui->pushButton_reset->setEnabled(true);
        break;

    case AppState::RUN_FINISHED:
        _ui->pushButton_reset->setEnabled(true);
        _ui->plainTextEdit_program->setReadOnly(false);
        _ui->pushButton_open->setEnabled(true);
        _ui->pushButton_save->setEnabled(true);
        break;
    }
}

void MainWindow::updateInputUIFromQueue()
{
    QStringList remaining;
    std::queue<int> temp = _inputTokens;
    while (!temp.empty()) {
        remaining << QString::number(temp.front());
        temp.pop();
    }

    _ui->plainTextEdit_input->blockSignals(true);
    _ui->plainTextEdit_input->setPlainText(remaining.join(" "));
    _ui->plainTextEdit_input->blockSignals(false);
}

bool MainWindow::feedInputIfAvailable()
{
    if (_computer.getCPU().isWaitingForInput() && !_inputTokens.empty())
    {
        int val = _inputTokens.front();
        _inputTokens.pop();

        _computer.provideInput(val);
        updateInputUIFromQueue();

        return true;
    }
    return false;
}

// put input tokens to the queue
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
            QMessageBox::critical(this, "Input Error", "Invalid input token: '" + str + "'. Must be a valid integer within +-" + QString::number(Config::OVERFLOW_LIMIT - 1));
            return false;
        }
        _inputTokens.push(val);
    }

    return true;
}

void MainWindow::onActionEnter()
{
    if (_currentState != AppState::WAITING_INPUT)
    {
        assert(false && "Enter action triggered in invalid state");
        return;
    }
    if (!_computer.getCPU().isWaitingForInput()) return;

    if (parseInputTokens() && !_inputTokens.empty())
    {
        int val = _inputTokens.front();
        _inputTokens.pop();

        _computer.provideInput(val);
        updateInputUIFromQueue();
        updateRegistersFlagsStats();

        changeState(_stateBeforeInput);
        highlightTableRowsCurrent();

        if (_currentState == AppState::RUNNING) {
            processRunChunk();
        }
        else if (_currentState == AppState::DEBUG_RUNNING)
        {
            _debugTimer.start();
        }
    }
}

void MainWindow::onActionRun()
{
    if (!(
        _currentState == AppState::IDLE ||
        _currentState == AppState::DEBUG_PAUSED
        ))
    {
        assert(false && "Run action triggered in invalid state");
        return;
    }

    if (!parseInputTokens())
    {
        return;
    }

    changeState(AppState::RUNNING);

    processRunChunk();
}

void MainWindow::onActionDebug()
{
    if (_currentState != AppState::IDLE)
    {
        assert(false && "Debug action triggered in invalid state");
        return;
    }

    if (!parseInputTokens())
    {
        return;
    }

    changeState(AppState::DEBUG_RUNNING);

    _debugTimer.start();
}

void MainWindow::makeComputerStep()
{
    try
    {
        StepResult result = _computer.step();

        if (_computer.getCPU().isWaitingForInput()) {
            feedInputIfAvailable();
        }

        appendNewOutput();
        updateRegistersFlagsStats();
        if (result.memoryWasWritten)
        {
            updateSingleMemoryRow(result.writtenAddress);
        }
        highlightTableRowsCurrent();

        int currentPc = _computer.getCPU().getProgramCounter();

        if (_computer.getCPU().isHalted())
        {
            _debugTimer.stop();
            changeState(AppState::RUN_FINISHED);
        }
        else if (_computer.getCPU().isWaitingForInput())
        {
            _ui->plainTextEdit_input->clear();
            _debugTimer.stop();
            changeState(AppState::WAITING_INPUT);
        }
        else if (_breakpoints.count(currentPc))
        {
            _debugTimer.stop();
            changeState(AppState::DEBUG_PAUSED);
        }
    }
    catch (const std::exception& e)
    {
        _debugTimer.stop();
        changeState(AppState::RUN_FINISHED);
        QMessageBox::critical(this, "Runtime Error", e.what());
    }
}

void MainWindow::onActionStep()
{
    if (!(_currentState == AppState::DEBUG_PAUSED ||
        _currentState == AppState::IDLE))
    {
        assert(false && "Step action triggered in invalid state");
        return;
    }

    if (_currentState == AppState::IDLE && !parseInputTokens())
    {
        return;
    }

    if (_currentState == AppState::IDLE)
    {
        changeState(AppState::DEBUG_PAUSED);
    }

    makeComputerStep();
}

void MainWindow::onActionContinue()
{
    if (_currentState != AppState::DEBUG_PAUSED)
    {
        assert(false && "Continue action triggered in invalid state");
        return;
    }

    changeState(AppState::DEBUG_RUNNING);

    _debugTimer.start();
}

void MainWindow::onActionStop()
{
    if (!(
        _currentState == AppState::RUNNING ||
        _currentState == AppState::DEBUG_RUNNING ||
        _currentState == AppState::DEBUG_PAUSED ||
        _currentState == AppState::WAITING_INPUT
        ))
    {
        assert(false && "Stop action triggered in invalid state");
        return;
    }

    switch (_currentState)
    {
    case AppState::RUNNING:
        changeState(AppState::RUN_FINISHED);
        break;
    case AppState::DEBUG_RUNNING:
        _debugTimer.stop();
        changeState(AppState::DEBUG_PAUSED);
        break;
    case AppState::DEBUG_PAUSED:
        changeState(AppState::RUN_FINISHED);
        break;
    case AppState::WAITING_INPUT:
        changeState(AppState::RUN_FINISHED);
        break;
    default:
        break;
    }
}

void MainWindow::onActionReset()
{
    if (!(
        _currentState == AppState::RUNNING ||
        _currentState == AppState::DEBUG_RUNNING ||
        _currentState == AppState::DEBUG_PAUSED ||
        _currentState == AppState::WAITING_INPUT ||
        _currentState == AppState::RUN_FINISHED
        ))
    {
        assert(false && "Reset action triggered in invalid state");
        return;
    }

    _debugTimer.stop();
    _computer.reset();
    _computer.loadProgram(_compiledBinary);

    _lastOutputSize = 0;
    _ui->plainTextEdit_output->clear();

    std::queue<int> empty;
    std::swap(_inputTokens, empty);

    updateFullMemoryTable();
    updateRegistersFlagsStats();
    changeState(AppState::IDLE);
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

void MainWindow::processRunChunk()
{
    if (_currentState != AppState::RUNNING)
    {
        assert(false && "Process run chunk triggered in invalid state");
        return;
    }

    int instructionsExecuted = 0;

    while (!_computer.getCPU().isHalted())
    {
        try
        {
            if (_computer.getCPU().isWaitingForInput()) {
                if (!feedInputIfAvailable()) {
                    break;
                }
            }

            _computer.step();
            ++instructionsExecuted;

            if (instructionsExecuted >= RUN_CHUNK_SIZE)
            {
                appendNewOutput();
                updateFullMemoryTable();
                highlightTableRowsCurrent();
                updateRegistersFlagsStats();

                QCoreApplication::processEvents();

                if (_currentState != AppState::RUNNING)
                {
                    return;
                }
                instructionsExecuted = 0;
            }
        }
        catch (const std::exception& e)
        {
            changeState(AppState::RUN_FINISHED);
            appendNewOutput();
            updateRegistersFlagsStats();
            updateFullMemoryTable();
            QMessageBox::critical(this, "Runtime Error", e.what());
            return;
        }
    }

    appendNewOutput();
    updateRegistersFlagsStats();
    updateFullMemoryTable();
    highlightTableRowsCurrent();

    if (_computer.getCPU().isWaitingForInput())
    {
        _ui->plainTextEdit_input->clear();
        changeState(AppState::WAITING_INPUT);
    }
    else if (_computer.getCPU().isHalted())
    {
        changeState(AppState::RUN_FINISHED);
    }
}

void MainWindow::onDebugTimerTick()
{
    if (_currentState != AppState::DEBUG_RUNNING)
    {
        _debugTimer.stop();
        assert(false && "Debug timer tick in invalid state");
        return;
    }

    makeComputerStep();
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