#pragma once

#include "UIUtils.h"
#include "Size.h"


// Animations are controlled by a Element's AnimationHandler
// Animations work by defining the intial state of the element and the target state,
// and during its update loop, the state of the element is being interpolated depening on how far along the animation is-
// When another Animation is queued, it iterrupts the current animation 


namespace ScaleformUI
{
	class IElement;
	class Element;

	using UIFunction = std::function<void(Element*)>;

	class AnimationHandler
	{
		class Animation;

		public:
			struct ScaleParameters
			{
				float scalePercentage = 1.0f;
				Alignment align1 = Alignment::kCenterH;
				Alignment align2 = Alignment::kCenterV;

				ScaleParameters(float a_scalePercentage) : scalePercentage(a_scalePercentage) {}
			};


			AnimationHandler(Element* a_element) : element(a_element) {}
			
			IElement*			GetElement() { return (IElement*)element; }
			std::vector<Animation>& GetAimationQueue() { return animationQueue; }

			void Update(float a_deltaTime);
			void EmptyAnimationQueue();
			void FinishAllAnimationsImmediately();
			void SetPostAnimationQueueFinishCallback(UIFunction a_callback, bool a_ensureRun = false);

			bool HasActiveAnimation() const { return !animationQueue.empty(); }
			bool IsAnimated() const { return currentState ? true : false; }

			Animation* PlayScaleFromAnimation(float a_duration, float a_scalePercentage);
			Animation* PlayScaleToAnimation(float a_duration, float a_scalePercentage);
			Animation* PlayAlignedScaleAnimation(float a_duration, ScaleParameters a_scaleParmaters);
			Animation* PlayAlignedScaleAnimation(float a_duration, float a_scalePercentage) { return PlayAlignedScaleAnimation(a_duration, ScaleParameters{ a_scalePercentage }); }
			Animation* PlayMoveToAnimation(float a_duration, Size a_x, Size a_y);
			Animation* PlayMoveFromAnimation(float a_duration, Size a_x, Size a_y);
			Animation* PlayMoveToAnimation(float a_duration, float a_x, float a_y) { return PlayMoveFromAnimation(a_duration, Size(a_x), Size(a_y)); }
			Animation* PlayMoveFromAnimation(float a_duration, float a_x, float a_y) { return PlayMoveFromAnimation(a_duration, Size(a_x), Size(a_y)); }
			Animation* PlayFromAlphaAnimation(float a_duration, uint32_t a_alpha);
			Animation* PlayToAlphaAnimation(float a_duration, uint32_t a_alpha);
			Animation* PlayResetAnimation(float a_duration);

			Animation* QueueScaleFromAnimation(float a_duration, float a_scalePercentage);
			Animation* QueueScaleToAnimation(float a_duration, float a_scalePercentage);
			Animation* QueueAlignedScaleAnimation(float a_duration, ScaleParameters a_scaleParmaters);
			Animation* QueueAlignedScaleAnimation(float a_duration, float a_scalePercentage) { return QueueAlignedScaleAnimation(a_duration, ScaleParameters{ a_scalePercentage }); }
			Animation* QueueMoveToAnimation(float a_duration, Size a_x, Size a_y);
			Animation* QueueMoveFromAnimation(float a_duration, Size a_x, Size a_y);
			Animation* QueueMoveToAnimation(float a_duration, float a_x, float a_y) { return QueueMoveFromAnimation(a_duration, Size(a_x), Size(a_y)); }
			Animation* QueueMoveFromAnimation(float a_duration, float a_x, float a_y) { return QueueMoveFromAnimation(a_duration, Size(a_x), Size(a_y)); }
			Animation* QueueFromAlphaAnimation(float a_duration, uint32_t a_alpha);
			Animation* QueueToAlphaAnimation(float a_duration, uint32_t a_alpha);
			Animation* QueueResetAnimation(float a_duration);

		private:
			Element* element = nullptr;

			class State
			{
				public:
					bool hasPosition = false;
					bool hasScale = false;
					bool hasAlpha = false;
					bool hasRotation = false;

					State() {}
					void Reset(Element* a_element);
					void SetX(float a_x) { hasPosition = true; x = a_x; }
					void SetY(float a_y) { hasPosition = true; y = a_y; }
					void SetXScale(float a_x) { hasScale = true; xScale = a_x; }
					void SetYScale(float a_y) { hasScale = true; yScale = a_y; }
					void SetAlpha(float a_alpha) { hasAlpha = true; alpha = a_alpha; }
					void SetAngle(float a_angle) { hasRotation = true; angle = a_angle; }

					float GetX() const { return x; }
					float GetY() const { return y; }
					float GetXScale() const { return xScale; }
					float GetYScale() const { return yScale; }
					float GetAlpha() const { return alpha; }
					float GetRotation() const { return angle; }

					constexpr operator bool() const { return hasPosition || hasScale || hasAlpha || hasRotation; }

					const bool operator==(State& a_other);

				private:
					float x = 0.0f;
					float y = 0.0f;
					float xScale = 1.0f;
					float yScale = 1.0f;
					uint32_t alpha = 100;
					float angle = 0.0f;
			};

			enum class InterpolationType
			{
				kLinear = 1
			};

		public:
			class Animation
			{
				public:
					Animation() {}
					void AddPostCallback(UIFunction a_callback) { postAnimationCallbacks.push_back(a_callback); }
					void RestoreElementOnAnimationFinish() { restoreElementOnAnimationFinish = true; }

				private:
					friend class AnimationHandler;

					State					initialState; // If initial state is not given, will use current state
					State					targetState; // If target state is not given, it will use the physical state. The has-flags will be set to that of the initial state
					float					duration = 0.0f;
					float					currentTime = 0.0f;
					std::vector<UIFunction>	postAnimationCallbacks;
					bool					restoreElementOnAnimationFinish = false;
					bool					playImmediately = false;
					InterpolationType		interpolationType = InterpolationType::kLinear;

					State InterpolateState();

			};
		
		private:

			std::vector<Animation>	animationQueue;
			UIFunction				postAnimationQueueFinishCallback = nullptr;
			bool					ensurePostAnimationQueueFinsihCallbackRuns = false; // can be used for vital callbacks, such as set visible on menu open
			State					currentState;

			void					PostAnimationQueueFinish();

			State		GetCurrentState();
			void		UpdateElementVisually(State a_state);
			void		OnAnimationStart(uint32_t a_animationIndex);
			void		OnAnimationFinish(uint32_t a_animationIndex);
			void		RestoreElement();
			void		RemoveFinishedAnimationsFromQueue();

			Animation	CreateScaleFromAnimation(float a_duration, float a_scalePercentage);
			Animation	CreateScaleToAnimation(float a_duration, float a_scalePercentage);
			Animation	CreateAlignedScaleAnimation(float a_duration, ScaleParameters a_scaleParameters);
			Animation	CreateMoveToAnimation(float a_duration, Size a_x, Size a_y);
			Animation	CreateMoveFromAnimation(float a_duration, Size a_x, Size a_y);
			Animation	CreateFromAlphaAnimation(float a_duration, uint32_t a_alpha);
			Animation	CreateToAlphaAnimation(float a_duration, uint32_t a_alpha);
			Animation	CreateResetAnimation(float a_duration);
			State		AlignedScaleToState(ScaleParameters a_scaleParameters);
			State		GetPhysicalState();
	};
}