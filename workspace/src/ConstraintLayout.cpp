#include "ConstraintLayout.hpp"
#include "SDLManager.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <cmath>

namespace TG5040
{
    namespace UI
    {

        // Constraint implementation
        float Constraint::getValue() const
        {
            if (!secondItem)
            {
                return constant;
            }

            float secondValue = secondItem->getConstraintValue(secondAttribute);
            return multiplier * secondValue + constant;
        }

        bool Constraint::isValid() const
        {
            return firstItem != nullptr;
        }

        // Element implementation
        Element::Element(const std::string &tag) : tag_(tag)
        {
        }

        void Element::addChild(ElementPtr child)
        {
            if (!child || child->parent_ == this)
            {
                return;
            }

            // Remove from previous parent
            if (child->parent_)
            {
                child->removeFromParent();
            }

            children_.push_back(child);
            child->parent_ = this;
            setNeedsLayout();
        }

        void Element::removeChild(ElementPtr child)
        {
            auto it = std::find(children_.begin(), children_.end(), child);
            if (it != children_.end())
            {
                (*it)->parent_ = nullptr;
                children_.erase(it);
                setNeedsLayout();
            }
        }

        void Element::removeFromParent()
        {
            if (parent_)
            {
                auto self = shared_from_this();
                parent_->removeChild(self);
            }
        }

        Container *Element::superview() const
        {
            return dynamic_cast<Container *>(parent_);
        }

        ConstraintPtr Element::leftAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Left, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Left, 1.0f, 0.0f);
        }

        ConstraintPtr Element::rightAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Right, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Right, 1.0f, 0.0f);
        }

        ConstraintPtr Element::topAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Top, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Top, 1.0f, 0.0f);
        }

        ConstraintPtr Element::bottomAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Bottom, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Bottom, 1.0f, 0.0f);
        }

        ConstraintPtr Element::widthAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Width, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Width, 1.0f, 0.0f);
        }

        ConstraintPtr Element::heightAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Height, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Height, 1.0f, 0.0f);
        }

        ConstraintPtr Element::centerXAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::CenterX, ConstraintRelation::Equal, nullptr, ConstraintAttribute::CenterX, 1.0f, 0.0f);
        }

        ConstraintPtr Element::centerYAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::CenterY, ConstraintRelation::Equal, nullptr, ConstraintAttribute::CenterY, 1.0f, 0.0f);
        }

        ConstraintPtr Element::leadingAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Leading, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Leading, 1.0f, 0.0f);
        }

        ConstraintPtr Element::trailingAnchor()
        {
            return std::make_shared<Constraint>(this, ConstraintAttribute::Trailing, ConstraintRelation::Equal, nullptr, ConstraintAttribute::Trailing, 1.0f, 0.0f);
        }

        void Element::addConstraint(ConstraintPtr constraint)
        {
            if (constraint && constraint->isValid())
            {
                constraints_.push_back(constraint);
                setNeedsLayout();
            }
        }

        void Element::addConstraints(const std::vector<ConstraintPtr> &constraints)
        {
            for (auto &constraint : constraints)
            {
                addConstraint(constraint);
            }
        }

        void Element::removeConstraint(ConstraintPtr constraint)
        {
            auto it = std::find(constraints_.begin(), constraints_.end(), constraint);
            if (it != constraints_.end())
            {
                constraints_.erase(it);
                setNeedsLayout();
            }
        }

        void Element::removeAllConstraints()
        {
            constraints_.clear();
            setNeedsLayout();
        }

        void Element::layoutSubviews()
        {
            // Layout children
            for (auto &child : children_)
            {
                if (child->needsLayout())
                {
                    child->layoutSubviews();
                }
            }
            needsLayout_ = false;
        }

        void Element::render(SDL_Renderer *renderer)
        {
            renderBackground(renderer);
            renderContent(renderer);
            renderBorder(renderer);

            // Render children
            for (auto &child : children_)
            {
                child->render(renderer);
            }
        }

        void Element::renderBackground(SDL_Renderer *renderer)
        {
            if (backgroundColor.a > 0)
            {
                SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
                SDL_Rect rect = frame.toSDL();
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        void Element::renderBorder(SDL_Renderer *renderer)
        {
            if (borderWidth > 0)
            {
                SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
                SDL_Rect rect = frame.toSDL();

                for (int i = 0; i < static_cast<int>(borderWidth); ++i)
                {
                    SDL_RenderDrawRect(renderer, &rect);
                    rect.x += 1;
                    rect.y += 1;
                    rect.w -= 2;
                    rect.h -= 2;
                }
            }
        }

        float Element::getConstraintValue(ConstraintAttribute attribute) const
        {
            switch (attribute)
            {
            case ConstraintAttribute::Left:
            case ConstraintAttribute::Leading:
                return frame.x;
            case ConstraintAttribute::Right:
            case ConstraintAttribute::Trailing:
                return frame.x + frame.width;
            case ConstraintAttribute::Top:
                return frame.y;
            case ConstraintAttribute::Bottom:
                return frame.y + frame.height;
            case ConstraintAttribute::Width:
                return frame.width;
            case ConstraintAttribute::Height:
                return frame.height;
            case ConstraintAttribute::CenterX:
                return frame.x + frame.width * 0.5f;
            case ConstraintAttribute::CenterY:
                return frame.y + frame.height * 0.5f;
            default:
                return 0.0f;
            }
        }

        void Element::setConstraintValue(ConstraintAttribute attribute, float value)
        {
            switch (attribute)
            {
            case ConstraintAttribute::Left:
            case ConstraintAttribute::Leading:
                frame.x = value;
                break;
            case ConstraintAttribute::Right:
            case ConstraintAttribute::Trailing:
                frame.x = value - frame.width;
                break;
            case ConstraintAttribute::Top:
                frame.y = value;
                break;
            case ConstraintAttribute::Bottom:
                frame.y = value - frame.height;
                break;
            case ConstraintAttribute::Width:
                frame.width = value;
                break;
            case ConstraintAttribute::Height:
                frame.height = value;
                break;
            case ConstraintAttribute::CenterX:
                frame.x = value - frame.width * 0.5f;
                break;
            case ConstraintAttribute::CenterY:
                frame.y = value - frame.height * 0.5f;
                break;
            }
        }

        // Container implementation
        void Container::computeConstraints()
        {
            solveConstraints();
        }

        void Container::layoutSubviews()
        {
            if (needsLayout())
            {
                computeConstraints();
            }
            Element::layoutSubviews();
        }

        void Container::solveConstraints()
        {
            // Simple constraint solver - processes constraints in order
            // For a production system, you'd want a more sophisticated solver

            for (auto &child : children_)
            {
                for (auto &constraint : child->constraints_)
                {
                    if (!constraint->active || !constraint->isValid())
                    {
                        continue;
                    }

                    float value = constraint->getValue();
                    child->setConstraintValue(constraint->firstAttribute, value);
                }
            }
        }

        // Text implementation
        Text::Text(const std::string &text, int fontSize)
            : Element("text"), text_(text), fontSize_(fontSize)
        {
            calculateTextSize();
        }

        void Text::setText(const std::string &text)
        {
            if (text_ != text)
            {
                text_ = text;
                calculateTextSize();
                setNeedsLayout();
            }
        }

        void Text::setFontSize(int size)
        {
            if (fontSize_ != size)
            {
                fontSize_ = size;
                calculateTextSize();
                setNeedsLayout();
            }
        }

        void Text::setFontPath(const std::string &fontPath)
        {
            if (fontPath_ != fontPath)
            {
                fontPath_ = fontPath;
                calculateTextSize();
                setNeedsLayout();
            }
        }

        void Text::renderContent(SDL_Renderer *renderer)
        {
            if (text_.empty())
            {
                return;
            }

            // Get font from SDLManager
            TTF_Font *font = nullptr;
            if (fontPath_.empty())
            {
                font = SDLManager::getInstance().getDefaultFont(fontSize_);
            }
            else
            {
                font = SDLManager::getInstance().getFont(fontPath_, fontSize_);
            }

            if (!font)
            {
                // Fallback to simple rectangle rendering if font loading fails
                SDL_SetRenderDrawColor(renderer, textColor_.r, textColor_.g, textColor_.b, textColor_.a);

                float charWidth = fontSize_ * 0.6f;
                float x = frame.x + 5;
                float y = frame.y + 5;

                for (size_t i = 0; i < text_.length() && x < frame.x + frame.width - charWidth; ++i)
                {
                    SDL_Rect charRect = {
                        static_cast<int>(x),
                        static_cast<int>(y),
                        static_cast<int>(charWidth * 0.8f),
                        static_cast<int>(fontSize_)};
                    SDL_RenderDrawRect(renderer, &charRect);
                    x += charWidth;
                }
                return;
            }

            // Render actual text using TTF
            SDL_Color sdlColor = textColor_.toSDL();
            SDL_Surface *textSurface = TTF_RenderText_Blended(font, text_.c_str(), sdlColor);

            if (!textSurface)
            {
                LOG_ERROR("Failed to render text surface: %s", TTF_GetError());
                return;
            }

            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);

            if (!textTexture)
            {
                LOG_ERROR("Failed to create text texture: %s", SDL_GetError());
                return;
            }

            // Get text dimensions
            int textWidth, textHeight;
            SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

            // Center text in the frame
            SDL_Rect destRect = {
                static_cast<int>(frame.x + (frame.width - textWidth) * 0.5f),
                static_cast<int>(frame.y + (frame.height - textHeight) * 0.5f),
                textWidth,
                textHeight};

            SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
            SDL_DestroyTexture(textTexture);
        }

        void Text::calculateTextSize()
        {
            // Try to get actual text dimensions using font
            TTF_Font *font = nullptr;
            if (fontPath_.empty())
            {
                font = SDLManager::getInstance().getDefaultFont(fontSize_);
            }
            else
            {
                font = SDLManager::getInstance().getFont(fontPath_, fontSize_);
            }

            if (font && !text_.empty())
            {
                int textWidth, textHeight;
                if (TTF_SizeText(font, text_.c_str(), &textWidth, &textHeight) == 0)
                {
                    frame.width = textWidth + 10;   // 5px padding on each side
                    frame.height = textHeight + 10; // 5px padding top/bottom
                    return;
                }
            }

            // Fallback to simple calculation
            float charWidth = fontSize_ * 0.6f;
            frame.width = text_.length() * charWidth + 10; // 5px padding on each side
            frame.height = fontSize_ + 10;                 // 5px padding top/bottom
        }

        // Button implementation
        Button::Button(const std::string &title) : Element("button"), title_(title)
        {
            backgroundColor = normalColor_;
        }

        void Button::setTitle(const std::string &title)
        {
            title_ = title;
            setNeedsLayout();
        }

        bool Button::handleEvent(const SDL_Event &event)
        {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                if (frame.contains(event.button.x, event.button.y))
                {
                    isPressed_ = true;
                    backgroundColor = pressedColor_;
                    return true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                if (isPressed_)
                {
                    isPressed_ = false;
                    backgroundColor = normalColor_;

                    if (frame.contains(event.button.x, event.button.y) && onClickCallback_)
                    {
                        onClickCallback_();
                    }
                    return true;
                }
            }

            return Element::handleEvent(event);
        }

        void Button::renderContent(SDL_Renderer *renderer)
        {
            if (title_.empty())
            {
                return;
            }

            // Simple button text rendering
            SDL_SetRenderDrawColor(renderer, textColor_.r, textColor_.g, textColor_.b, textColor_.a);

            float charWidth = fontSize_ * 0.6f;
            float textWidth = title_.length() * charWidth;
            float x = frame.x + (frame.width - textWidth) * 0.5f;
            float y = frame.y + (frame.height - fontSize_) * 0.5f;

            for (size_t i = 0; i < title_.length(); ++i)
            {
                SDL_Rect charRect = {
                    static_cast<int>(x),
                    static_cast<int>(y),
                    static_cast<int>(charWidth * 0.8f),
                    static_cast<int>(fontSize_)};
                SDL_RenderFillRect(renderer, &charRect);
                x += charWidth;
            }
        }

        // Image implementation
        Image::Image(const std::string &imagePath) : Element("image"), imagePath_(imagePath)
        {
        }

        void Image::setImagePath(const std::string &path)
        {
            if (imagePath_ != path)
            {
                imagePath_ = path;
                texture_ = nullptr; // Will be reloaded on next render
                setNeedsLayout();
            }
        }

        void Image::renderContent(SDL_Renderer *renderer)
        {
            if (imagePath_.empty())
            {
                return;
            }

            if (!texture_)
            {
                loadTexture(renderer);
            }

            if (texture_)
            {
                SDL_Rect destRect = frame.toSDL();
                SDL_RenderCopy(renderer, texture_, nullptr, &destRect);
            }
            else
            {
                // Fallback: draw a placeholder rectangle
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                SDL_Rect rect = frame.toSDL();
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

        void Image::loadTexture(SDL_Renderer *renderer)
        {
            // This would load the actual image using SDL_image
            // For now, just create a simple colored texture as placeholder
            texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         static_cast<int>(frame.width),
                                         static_cast<int>(frame.height));

            if (texture_)
            {
                SDL_SetRenderTarget(renderer, texture_);
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderClear(renderer);
                SDL_SetRenderTarget(renderer, nullptr);
            }
        }

    } // namespace UI
} // namespace TG5040
