#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>

namespace TG5040
{
    namespace UI
    {

        // Forward declarations
        class Element;
        class Container;
        using ElementPtr = std::shared_ptr<Element>;

        // Color structure
        struct Color
        {
            uint8_t r = 0, g = 0, b = 0, a = 255;

            Color() = default;
            Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
                : r(red), g(green), b(blue), a(alpha) {}

            SDL_Color toSDL() const { return {r, g, b, a}; }

            static Color white() { return {255, 255, 255, 255}; }
            static Color black() { return {0, 0, 0, 255}; }
            static Color red() { return {255, 0, 0, 255}; }
            static Color green() { return {0, 255, 0, 255}; }
            static Color blue() { return {0, 0, 255, 255}; }
            static Color transparent() { return {0, 0, 0, 0}; }
        };

        // Rectangle structure
        struct Rect
        {
            float x = 0, y = 0, width = 0, height = 0;

            Rect() = default;
            Rect(float x_, float y_, float w, float h) : x(x_), y(y_), width(w), height(h) {}

            bool contains(float px, float py) const
            {
                return px >= x && px < x + width && py >= y && py < y + height;
            }

            SDL_Rect toSDL() const
            {
                return {static_cast<int>(x), static_cast<int>(y),
                        static_cast<int>(width), static_cast<int>(height)};
            }
        };

        // Constraint types - similar to iOS Auto Layout
        enum class ConstraintAttribute
        {
            Left,
            Right,
            Top,
            Bottom,
            Width,
            Height,
            CenterX,
            CenterY,
            Leading,
            Trailing
        };

        enum class ConstraintRelation
        {
            Equal,
            LessThanOrEqual,
            GreaterThanOrEqual
        };

        // Layout priority (higher = more important)
        enum class LayoutPriority
        {
            Required = 1000,
            DefaultHigh = 750,
            DefaultLow = 250
        };

        // Constraint class - similar to NSLayoutConstraint
        class Constraint
        {
        public:
            Element *firstItem = nullptr;
            ConstraintAttribute firstAttribute = ConstraintAttribute::Left;
            ConstraintRelation relation = ConstraintRelation::Equal;
            Element *secondItem = nullptr;
            ConstraintAttribute secondAttribute = ConstraintAttribute::Left;
            float multiplier = 1.0f;
            float constant = 0.0f;
            LayoutPriority priority = LayoutPriority::Required;
            bool active = true;

            Constraint(Element *item1, ConstraintAttribute attr1, ConstraintRelation rel,
                       Element *item2, ConstraintAttribute attr2, float mult = 1.0f, float c = 0.0f)
                : firstItem(item1), firstAttribute(attr1), relation(rel),
                  secondItem(item2), secondAttribute(attr2), multiplier(mult), constant(c) {}

            // Convenience constructor for constant constraints
            Constraint(Element *item, ConstraintAttribute attr, ConstraintRelation rel, float c)
                : firstItem(item), firstAttribute(attr), relation(rel),
                  secondItem(nullptr), secondAttribute(attr), multiplier(1.0f), constant(c) {}

            float getValue() const;
            bool isValid() const;

            // Allow Container and Element to access constraint internals
            friend class Element;
            friend class Container;
        };

        using ConstraintPtr = std::shared_ptr<Constraint>;

        // Base UI Element with constraint-based layout
        class Element : public std::enable_shared_from_this<Element>
        {
        public:
            Element(const std::string &tag = "div");
            virtual ~Element() = default;

            // Layout properties
            Rect frame; // Final computed frame
            bool translatesAutoresizingMaskIntoConstraints = true;

            // Visual properties
            Color backgroundColor = Color::transparent();
            float cornerRadius = 0.0f;
            float borderWidth = 0.0f;
            Color borderColor = Color::black();

            // Hierarchy
            void addChild(ElementPtr child);
            void removeChild(ElementPtr child);
            void removeFromParent();
            const std::vector<ElementPtr> &children() const { return children_; }
            Element *parent() const { return parent_; }
            Container *superview() const;

            // Constraints - iOS-style API
            ConstraintPtr leftAnchor();
            ConstraintPtr rightAnchor();
            ConstraintPtr topAnchor();
            ConstraintPtr bottomAnchor();
            ConstraintPtr widthAnchor();
            ConstraintPtr heightAnchor();
            ConstraintPtr centerXAnchor();
            ConstraintPtr centerYAnchor();
            ConstraintPtr leadingAnchor();
            ConstraintPtr trailingAnchor();

            // Constraint convenience methods
            void addConstraint(ConstraintPtr constraint);
            void addConstraints(const std::vector<ConstraintPtr> &constraints);
            void removeConstraint(ConstraintPtr constraint);
            void removeAllConstraints();

            // Layout
            virtual void layoutSubviews();
            void setNeedsLayout() { needsLayout_ = true; }
            bool needsLayout() const { return needsLayout_; }

            // Events
            virtual bool handleEvent(const SDL_Event &event) { return false; }

            // Rendering
            virtual void render(SDL_Renderer *renderer);

            // Identification
            void setTag(const std::string &tag) { tag_ = tag; }
            const std::string &tag() const { return tag_; }

            // Allow Container to access constraint internals
            friend class Container;
            friend class Constraint;

        public: // Make these public for constraint system
            float getConstraintValue(ConstraintAttribute attribute) const;
            void setConstraintValue(ConstraintAttribute attribute, float value);

        protected:
            std::string tag_;
            std::vector<ElementPtr> children_;
            Element *parent_ = nullptr;
            bool needsLayout_ = true;

        public: // Make constraints public for constraint system
            std::vector<ConstraintPtr> constraints_;

        protected:
            virtual void renderBackground(SDL_Renderer *renderer);
            virtual void renderContent(SDL_Renderer *renderer) {}
            virtual void renderBorder(SDL_Renderer *renderer);
        };

        // Container element
        class Container : public Element
        {
        public:
            Container() : Element("container") {}

            // Layout computation
            void computeConstraints();
            void layoutSubviews() override;

        protected:
            void solveConstraints();
        };

        // Text element
        class Text : public Element
        {
        public:
            Text(const std::string &text = "", int fontSize = 16);

            void setText(const std::string &text);
            const std::string &getText() const { return text_; }

            void setFontSize(int size);
            int getFontSize() const { return fontSize_; }

            void setTextColor(const Color &color) { textColor_ = color; }
            const Color &getTextColor() const { return textColor_; }

            void setFontPath(const std::string &fontPath);
            const std::string &getFontPath() const { return fontPath_; }

        protected:
            void renderContent(SDL_Renderer *renderer) override;

        private:
            std::string text_;
            int fontSize_ = 16;
            Color textColor_ = Color::black();
            std::string fontPath_; // Empty means use default font

            void calculateTextSize();
        };

        // Button element
        class Button : public Element
        {
        public:
            Button(const std::string &title = "");

            void setTitle(const std::string &title);
            const std::string &getTitle() const { return title_; }

            void setOnClick(std::function<void()> callback) { onClickCallback_ = callback; }

            bool handleEvent(const SDL_Event &event) override;

        protected:
            void renderContent(SDL_Renderer *renderer) override;

        private:
            std::string title_;
            std::function<void()> onClickCallback_;
            bool isPressed_ = false;
            Color normalColor_ = Color(100, 100, 100);
            Color pressedColor_ = Color(80, 80, 80);
            Color textColor_ = Color::white();
            int fontSize_ = 16;
        };

        // Image element
        class Image : public Element
        {
        public:
            Image(const std::string &imagePath = "");

            void setImagePath(const std::string &path);
            const std::string &getImagePath() const { return imagePath_; }

        protected:
            void renderContent(SDL_Renderer *renderer) override;

        private:
            std::string imagePath_;
            SDL_Texture *texture_ = nullptr;

            void loadTexture(SDL_Renderer *renderer);
        };

    } // namespace UI
} // namespace TG5040
