#pragma once
#include <QMainWindow>
#include <QString>
#include <QCloseEvent>
#include <QTimer>
#include <unordered_set>
#include <queue>
#include "../core/Computer.h"

namespace Ui
{
    class MainWindow;
}

class CpuStats;

enum class EmulationMode { STOPPED, RUNNING, DEBUGGING };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::MainWindow* _ui;
    CpuStats* _stats;
    bool _hasUnsavedChanges;
    bool _hasUncompiledChanges;
    QString _currentFilePath;
    std::vector<int> _compiledBinary;
    std::unordered_map<int, std::string> _currentLabels;
    Computer _computer;

    EmulationMode _currentMode;
    QTimer _debugTimer;
    std::unordered_set<int> _breakpoints;
    std::queue<int> _inputTokens;
    size_t _lastOutputSize;

    void setupMemoryTables();
    void updateUI(bool fullUpdate = false);
    void setupConnections();
    bool promptSaveIfUnsaved();

    void setUiInteractionEnabled(bool enabled);
    void highlightCurrentInstruction();
    bool parseInputTokens();
    void processRunChunk();
    void appendNewOutput();
    void toggleBreakpoint(int row);

    void onActionOpen();
    void onActionSave();
    void onActionCompile();
    void onProgramTextChanged();

    void onActionLoad();
    void onActionRun();
    void onActionDebug();
    void onActionStep();
    void onActionStop();
    void onActionReset();
    void onActionEnter();

    void onDebugTimerTick();
    void onMemoryTableClicked(int row, int column);
    void onMinStepDurationChanged(const QString& value);
    void onMinStepDurationSliderChanged(int value);
};