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
#include "../assembler/Assembler.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow{ parent },
    _ui{ new Ui::MainWindow },
    _stats{ new CpuStats{ this } },
    _hasUnsavedChanges{ false },
    _currentFilePath{ "" }
{
    _ui->setupUi(this);

    _ui->checkBox_overflow->setAttribute(Qt::WA_TransparentForMouseEvents);
    _ui->checkBox_overflow->setFocusPolicy(Qt::NoFocus);

    _ui->splitter_main->setStretchFactor(0, 3);
    _ui->splitter_main->setStretchFactor(1, 3);
    _ui->splitter_main->setStretchFactor(2, 2);

    setupMemoryTable();

    _ui->pieChartWidget_instructionDistribution->rootContext()->setContextProperty("cpuData", _stats);

    QSurfaceFormat format;
    format.setSamples(8);
    _ui->pieChartWidget_instructionDistribution->setFormat(format);
    _ui->pieChartWidget_instructionDistribution->setResizeMode(QQuickWidget::SizeRootObjectToView);
    _ui->pieChartWidget_instructionDistribution->setSource(QUrl(QStringLiteral("qrc:/qml/PieChart.qml")));

    setupConnections();
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::setupMemoryTable()
{
    _ui->tableMemory->setColumnCount(4);
    _ui->tableMemory->setHorizontalHeaderLabels({ " BP ", " Address ", " Data ", " Data Decoded " });
    _ui->tableMemory->setColumnWidth(0, 30);
    _ui->tableMemory->setColumnWidth(1, 60);
    _ui->tableMemory->setColumnWidth(2, 80);
    _ui->tableMemory->horizontalHeader()->setStretchLastSection(true);
    _ui->tableMemory->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _ui->tableMemory->setSelectionBehavior(QAbstractItemView::SelectRows);
    _ui->tableMemory->verticalHeader()->setVisible(false);
    _ui->tableMemory->setRowCount(1000);

    QColor memoryCellColor(45, 45, 48);

    for (int i = 0; i < 1000; i++)
    {
        auto* itemBP = new QTableWidgetItem("");
        itemBP->setTextAlignment(Qt::AlignCenter);

        QString addrStr = QString::number(i).rightJustified(3, '0');
        auto* itemAddr = new QTableWidgetItem(addrStr);
        itemAddr->setTextAlignment(Qt::AlignCenter);
        itemAddr->setForeground(QBrush(QColor(133, 133, 133)));

        auto* itemData = new QTableWidgetItem("00000");
        itemData->setTextAlignment(Qt::AlignCenter);
        itemData->setBackground(QBrush(memoryCellColor));

        auto* itemCode = new QTableWidgetItem("NOP");

        _ui->tableMemory->setItem(i, 0, itemBP);
        _ui->tableMemory->setItem(i, 1, itemAddr);
        _ui->tableMemory->setItem(i, 2, itemData);
        _ui->tableMemory->setItem(i, 3, itemCode);
    }
}

void MainWindow::setupConnections()
{
    connect(_ui->pushButton_open, &QPushButton::clicked, this, &MainWindow::onActionOpen);
    connect(_ui->pushButton_save, &QPushButton::clicked, this, &MainWindow::onActionSave);
	connect(_ui->pushButton_compile, &QPushButton::clicked, this, &MainWindow::onActionCompile);

    connect(_ui->plainTextEdit_program, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
}

void MainWindow::onTextChanged()
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