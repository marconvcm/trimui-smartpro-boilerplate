#include "Application.hpp"
#include "ConstraintLayout.hpp"
#include "ControllerManager.hpp"
#include "Logger.hpp"
#include <memory>

using namespace TG5040;
using namespace TG5040::UI;

class ConstraintDemoApp : public Application
{
public:
    ConstraintDemoApp() : Application("TG5040 Constraint Layout Demo", 1280, 720) {}

protected:
    void onCreate() override
    {
        LOG_INFO("Creating Constraint Layout Demo UI");

        // Create UI hierarchy with constraints
        createUserInterface();

        // Setup controller callbacks
        setupControllers();

        // Initialize countdown
        countdownTime_ = 10.0f;
        startTime_ = SDL_GetTicks();
    }

    void onUpdate(float deltaTime) override
    {
        updateCountdown(deltaTime);
    }

    bool onEvent(const SDL_Event &event) override
    {
        // Handle keyboard events as fallback
        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                restartCountdown();
                return true;
            }
        }
        return false;
    }

private:
    float countdownTime_;
    Uint32 startTime_;

    // Exit state tracking
    bool exitScheduled_ = false;
    float exitTimer_ = 0.0f;

    std::shared_ptr<Container> mainContainer_;
    std::shared_ptr<Text> titleText_;
    std::shared_ptr<Text> countdownText_;
    std::shared_ptr<Text> instructionText_;
    std::shared_ptr<Button> restartButton_;
    std::shared_ptr<Button> quitButton_;

    void setupControllers()
    {
        auto &controller = ControllerManager::getInstance();

        // A button - restart countdown
        controller.onButtonPressed(GamepadButton::A, [this]() 
        {
            LOG_INFO("Controller A button pressed - restarting countdown");
            restartCountdown(); 
        });

        // B button - quit app
        controller.onButtonPressed(GamepadButton::B, [this]()
        {
            LOG_INFO("Controller B button pressed - quitting app");
            quit(); 
        });

        // DPAD for navigation (example)
        controller.onButtonPressed(GamepadButton::DPAD_UP, [this]() { LOG_INFO("DPAD UP pressed"); });

        controller.onButtonPressed(GamepadButton::DPAD_DOWN, [this]() { LOG_INFO("DPAD DOWN pressed"); });

        // Log all button events for debugging
        controller.onButtonEvent([](GamepadButton button, ButtonState state)
        {
            const char* stateStr = (state == ButtonState::Pressed) ? "pressed" : "released";
            LOG_DEBUG("Controller button %d %s", static_cast<int>(button), stateStr); 
        });
    }

    void createUserInterface()
    {
        // Create main container (root view)
        mainContainer_ = std::make_shared<Container>();
        mainContainer_->backgroundColor = Color(30, 30, 30); // Dark background

        // Create title text
        titleText_ = std::make_shared<Text>("TG5040 Constraint Demo", 36);
        titleText_->setTextColor(Color::white());
        titleText_->backgroundColor = Color(50, 50, 100, 100); // Semi-transparent blue

        // Create countdown text
        countdownText_ = std::make_shared<Text>("10", 72);
        countdownText_->setTextColor(Color(51, 102, 255));    // Blue
        countdownText_->backgroundColor = Color(0, 0, 0, 50); // Semi-transparent black

        // Create instruction text
        instructionText_ = std::make_shared<Text>("A: Restart | B: Quit | SPACE: Restart", 18);
        instructionText_->setTextColor(Color(200, 200, 200)); // Light gray

        // Create restart button
        restartButton_ = std::make_shared<Button>("Restart (A)");
        restartButton_->backgroundColor = Color(76, 175, 80); // Green
        restartButton_->setOnClick([this]()
                                   { restartCountdown(); });

        // Create quit button
        quitButton_ = std::make_shared<Button>("Quit (B)");
        quitButton_->backgroundColor = Color(244, 67, 54); // Red
        quitButton_->setOnClick([this]()
                                { quit(); });

        // Add children to container
        mainContainer_->addChild(titleText_);
        mainContainer_->addChild(countdownText_);
        mainContainer_->addChild(instructionText_);
        mainContainer_->addChild(restartButton_);
        mainContainer_->addChild(quitButton_);

        // Setup constraints - iOS Auto Layout style
        setupLayoutConstraints();

        // Set as root element
        setRootElement(mainContainer_);
    }

    void setupLayoutConstraints()
    {
        // Disable autoresizing mask translation for constraint-based layout
        titleText_->translatesAutoresizingMaskIntoConstraints = false;
        countdownText_->translatesAutoresizingMaskIntoConstraints = false;
        instructionText_->translatesAutoresizingMaskIntoConstraints = false;
        restartButton_->translatesAutoresizingMaskIntoConstraints = false;
        quitButton_->translatesAutoresizingMaskIntoConstraints = false;

        // Title text constraints - centered horizontally, 150pt from top
        auto titleCenterX = std::make_shared<Constraint>(
            titleText_.get(), ConstraintAttribute::CenterX, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::CenterX, 1.0f, 0.0f);
        auto titleTop = std::make_shared<Constraint>(
            titleText_.get(), ConstraintAttribute::Top, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::Top, 1.0f, 150.0f);
        titleText_->addConstraints({titleCenterX, titleTop});

        // Countdown text constraints - centered both ways
        auto countdownCenterX = std::make_shared<Constraint>(
            countdownText_.get(), ConstraintAttribute::CenterX, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::CenterX, 1.0f, 0.0f);
        auto countdownCenterY = std::make_shared<Constraint>(
            countdownText_.get(), ConstraintAttribute::CenterY, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::CenterY, 1.0f, 0.0f);
        countdownText_->addConstraints({countdownCenterX, countdownCenterY});

        // Instruction text constraints - centered horizontally, 100pt from bottom
        auto instructionCenterX = std::make_shared<Constraint>(
            instructionText_.get(), ConstraintAttribute::CenterX, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::CenterX, 1.0f, 0.0f);
        auto instructionBottom = std::make_shared<Constraint>(
            instructionText_.get(), ConstraintAttribute::Bottom, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::Bottom, 1.0f, -100.0f);
        instructionText_->addConstraints({instructionCenterX, instructionBottom});

        // Button constraints - side by side, centered horizontally as a group
        auto restartWidth = std::make_shared<Constraint>(
            restartButton_.get(), ConstraintAttribute::Width, ConstraintRelation::Equal,
            nullptr, ConstraintAttribute::Width, 1.0f, 150.0f);
        auto restartHeight = std::make_shared<Constraint>(
            restartButton_.get(), ConstraintAttribute::Height, ConstraintRelation::Equal,
            nullptr, ConstraintAttribute::Height, 1.0f, 50.0f);
        auto restartRight = std::make_shared<Constraint>(
            restartButton_.get(), ConstraintAttribute::Right, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::CenterX, 1.0f, -10.0f);
        auto restartBottom = std::make_shared<Constraint>(
            restartButton_.get(), ConstraintAttribute::Bottom, ConstraintRelation::Equal,
            instructionText_.get(), ConstraintAttribute::Top, 1.0f, -20.0f);
        restartButton_->addConstraints({restartWidth, restartHeight, restartRight, restartBottom});

        auto quitWidth = std::make_shared<Constraint>(
            quitButton_.get(), ConstraintAttribute::Width, ConstraintRelation::Equal,
            nullptr, ConstraintAttribute::Width, 1.0f, 150.0f);
        auto quitHeight = std::make_shared<Constraint>(
            quitButton_.get(), ConstraintAttribute::Height, ConstraintRelation::Equal,
            nullptr, ConstraintAttribute::Height, 1.0f, 50.0f);
        auto quitLeft = std::make_shared<Constraint>(
            quitButton_.get(), ConstraintAttribute::Left, ConstraintRelation::Equal,
            mainContainer_.get(), ConstraintAttribute::CenterX, 1.0f, 10.0f);
        auto quitBottom = std::make_shared<Constraint>(
            quitButton_.get(), ConstraintAttribute::Bottom, ConstraintRelation::Equal,
            instructionText_.get(), ConstraintAttribute::Top, 1.0f, -20.0f);
        quitButton_->addConstraints({quitWidth, quitHeight, quitLeft, quitBottom});
    }

    void updateCountdown(float deltaTime)
    {
        if (countdownTime_ > 0)
        {
            Uint32 currentTime = SDL_GetTicks();
            float elapsed = (currentTime - startTime_) / 1000.0f; // Convert to seconds
            countdownTime_ -= elapsed;
            startTime_ = currentTime; // Reset start time for next frame

            // Update countdown text
            int remainingSeconds = static_cast<int>(countdownTime_);
            countdownText_->setText(std::to_string(remainingSeconds));

            // Check if countdown has reached zero
            if (countdownTime_ <= 0)
            {
                LOG_INFO("Countdown finished");
                countdownText_->setText("Bye!");
                countdownText_->setTextColor(Color::red()); // Change color to red
                exitScheduled_ = true;                      // Schedule exit
                exitTimer_ = 2.0f;
            }
        }
        else if (exitScheduled_)
        {
            // Handle exit timer
            exitTimer_ -= deltaTime;
            if (exitTimer_ <= 0)
            {
                LOG_INFO("Exiting application after countdown");
                quit(); // Exit the application
            }
        }
    }

    void restartCountdown()
    {
        LOG_INFO("Restarting countdown");
        countdownTime_ = 10.0f;
        startTime_ = SDL_GetTicks();

        // Reset exit state
        exitScheduled_ = false;
        exitTimer_ = 0.0f;

        // Reset text appearance
        countdownText_->setText("10");
        countdownText_->setTextColor(Color(51, 102, 255)); // Blue
        countdownText_->setFontSize(72);
    }
};

int main()
{
    ConstraintDemoApp app;

    if (!app.initialize())
    {
        LOG_FATAL("Failed to initialize application");
        return 1;
    }

    app.run(); // Main application loop

    LOG_INFO("Application finished successfully");
    return 0;
}
