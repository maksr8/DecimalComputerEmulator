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

enum class AppState {
    EDITING,
    IDLE,
    RUNNING,
    DEBUG_RUNNING,
    DEBUG_PAUSED,
    WAITING_INPUT,
    RUN_FINISHED
};

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
    bool _hasUnsavedChanges{ false };
    bool _isBinaryUpToDate{ false };
    void setUnsavedChanges(bool value);
    void setBinaryUpToDate(bool value);
    AppState _currentState{ AppState::EDITING };
    AppState _stateBeforeInput{ AppState::IDLE };
    QString _currentFilePath{ "" };
    std::vector<int> _compiledBinary;
    std::unordered_map<int, std::string> _currentLabels;
    Computer _computer{};
    QTimer _debugTimer;
    std::unordered_set<int> _breakpoints;
    std::queue<int> _inputTokens;
    size_t _lastOutputSize{ 0 };

    void setupMemoryTables();
    void setupConnections();

    bool isProgramMutableState() const;
    void onProgramTextChanged();
    void onActionOpen();
    void onActionSave();
    void doSave();
    void onActionCompile();
    void onActionLoad();
    bool promptSaveIfUnsaved();

    void updateFullMemoryTable();
    void clearBreakpoints();
    void updateRegistersFlagsStats();
    void updateSingleMemoryRow(int address);
    void highlightTableRowsCurrent();
    void changeState(AppState newState);
    void applyStateToUI();

    void updateInputUIFromQueue();
    bool feedInputIfAvailable();
    bool parseInputTokens();
    void onActionEnter();
    void onActionRun();
    void onActionDebug();
    void makeComputerStep();
    void onActionStep();
    void onActionContinue();
    void onActionStop();
    void onActionReset();
    void appendNewOutput();

    void processRunChunk();
    void onDebugTimerTick();

    void onMemoryTableClicked(int row, int column);
    void toggleBreakpoint(int row);
    void onMinStepDurationChanged(const QString& value);
    void onMinStepDurationSliderChanged(int value);
};