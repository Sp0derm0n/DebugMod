#include "AnimationHandler.h"
#include "Element.h"

void ScaleformUI::AnimationHandler::State::Reset(Element* a_element)
{
	x = a_element->GetX();
	y = a_element->GetY();
	xScale = a_element->GetXScale();
	yScale = a_element->GetYScale();
	alpha = a_element->GetAlpha();
	angle = 0.0f;
	hasPosition = false;
	hasScale = false;
	hasAlpha = false;
	hasRotation = false;
}

void ScaleformUI::AnimationHandler::EmptyAnimationQueue()
{ 
	animationQueue.clear(); 
	if (ensurePostAnimationQueueFinsihCallbackRuns)
	{
		PostAnimationQueueFinish();
	}
}

const bool ScaleformUI::AnimationHandler::State::operator==(ScaleformUI::AnimationHandler::State& a_other)
{
	float eps = 1e-3f;
	if (std::abs(x-a_other.x) > eps) return false;
	if (std::abs(y-a_other.y) > eps) return false;
	if (std::abs(xScale-a_other.xScale) > eps) return false;
	if (std::abs(yScale-a_other.yScale) > eps) return false;
	if (alpha != a_other.alpha) return false;
	if (std::abs(angle-a_other.angle) > eps) return false;

	return true;
}

void ScaleformUI::AnimationHandler::Update(float a_deltaTime)
{
	if (!HasActiveAnimation()) return;
	uint32_t animationCount = animationQueue.size();
	for (int i = 0; i < animationCount; i++)
	{
		auto* animation = &animationQueue[i];
		bool advanceAnimation = i == 0 || animation->playImmediately;
		if (advanceAnimation)
		{
			if (animation->currentTime == 0.0f)
			{
				OnAnimationStart(i);
			}
			animation->currentTime += a_deltaTime;
			if (animation->currentTime >= animation->duration)
			{
				OnAnimationFinish(i);
				if (animationQueue.empty()) return;

				continue;
			}


			auto nextState = animation->InterpolateState();

			UpdateElementVisually(nextState);
		}
	}

	// clean up finished animations
	RemoveFinishedAnimationsFromQueue();
}

void ScaleformUI::AnimationHandler::FinishAllAnimationsImmediately()
{
	for (int i = 0; i < animationQueue.size(); i++)
	{
		animationQueue[i].duration = 0.0f;
		OnAnimationFinish(i);
	}
	RemoveFinishedAnimationsFromQueue();
}

void ScaleformUI::AnimationHandler::RemoveFinishedAnimationsFromQueue()
{
	for (size_t i = animationQueue.size(); i--;)
	{
		if (animationQueue[i].currentTime >= animationQueue[i].duration)
			animationQueue.erase(animationQueue.begin() + i);
	}
	if (animationQueue.size() == 0)
	{
		PostAnimationQueueFinish();
	}
}

void ScaleformUI::AnimationHandler::OnAnimationStart(uint32_t a_animationIndex)
{
	Animation& animation = animationQueue[a_animationIndex];
	if (!animation.initialState)
	{
		animation.initialState = GetCurrentState();
	}
	if (!animation.targetState)
	{
		animation.targetState = GetPhysicalState();

		animation.targetState.hasPosition = animation.initialState.hasPosition;
		animation.targetState.hasScale = animation.initialState.hasScale;
		animation.targetState.hasAlpha = animation.initialState.hasAlpha;
		animation.targetState.hasRotation = animation.initialState.hasRotation;
	}
	if (animation.initialState == animation.targetState)
	{
		animation.duration = 0.0f;
	}
}

void ScaleformUI::AnimationHandler::OnAnimationFinish(uint32_t a_animationIndex)
{
	Animation* animation = &animationQueue[a_animationIndex];
	
	for (const auto& callback : animation->postAnimationCallbacks)
	{
		callback(element);
	}

	if (animation->restoreElementOnAnimationFinish)
	{
		RestoreElement(); // Deletes animationQueue
		return;
	}
	else
	{
		UpdateElementVisually(animation->targetState);
	}
}

void ScaleformUI::AnimationHandler::RestoreElement()
{
	UpdateElementVisually(GetPhysicalState());
	animationQueue.clear();
	currentState.Reset(element);
}

ScaleformUI::AnimationHandler::State ScaleformUI::AnimationHandler::GetCurrentState()
{
	if (!currentState) 
	{
		currentState = GetPhysicalState();
	}
	return currentState;
}


void ScaleformUI::AnimationHandler::UpdateElementVisually(State a_state)
{
	if (!currentState) currentState = GetPhysicalState();
	if (a_state.hasPosition)
	{
		element->SetXImpl(a_state.GetX(), false);
		element->SetYImpl(a_state.GetY(), false);
		currentState.SetX(a_state.GetX());
		currentState.SetY(a_state.GetY());
	}
	if (a_state.hasScale)
	{
		element->SetXScaleImpl(a_state.GetXScale(), false);
		element->SetYScaleImpl(a_state.GetYScale(), false);
		currentState.SetXScale(a_state.GetXScale());
		currentState.SetYScale(a_state.GetYScale());
	}
	if (a_state.hasAlpha)
	{
		element->SetAlphaImpl(a_state.GetAlpha(), false);
		currentState.SetAlpha(a_state.GetAlpha());
	}
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayScaleFromAnimation(float a_duration, float a_scalePercentage)
{
	animationQueue.emplace_back(CreateScaleFromAnimation(a_duration, a_scalePercentage));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayScaleToAnimation(float a_duration, float a_scalePercentage)
{
	animationQueue.emplace_back(CreateScaleToAnimation(a_duration, a_scalePercentage));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayAlignedScaleAnimation(float a_duration, ScaleParameters a_scaleParmaters)
{
	animationQueue.emplace_back(CreateAlignedScaleAnimation(a_duration, a_scaleParmaters));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayMoveToAnimation(float a_duration, Size a_x, Size a_y)
{
	animationQueue.emplace_back(CreateMoveToAnimation(a_duration, a_x, a_y));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayMoveFromAnimation(float a_duration, Size a_x, Size a_y)
{
	animationQueue.emplace_back(CreateMoveFromAnimation(a_duration, a_x, a_y));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayFromAlphaAnimation(float a_duration, uint32_t a_alpha)
{
	animationQueue.emplace_back(CreateFromAlphaAnimation(a_duration, a_alpha));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayToAlphaAnimation(float a_duration, uint32_t a_alpha)
{
	animationQueue.emplace_back(CreateToAlphaAnimation(a_duration, a_alpha));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::PlayResetAnimation(float a_duration)
{
	animationQueue.emplace_back(CreateResetAnimation(a_duration));
	animationQueue.back().playImmediately = true;
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueScaleFromAnimation(float a_duration, float a_scalePercentage)
{
	animationQueue.emplace_back(CreateScaleFromAnimation(a_duration, a_scalePercentage));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueScaleToAnimation(float a_duration, float a_scalePercentage)
{
	animationQueue.emplace_back(CreateScaleToAnimation(a_duration, a_scalePercentage));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueAlignedScaleAnimation(float a_duration, ScaleParameters a_scaleParmaters)
{
	animationQueue.emplace_back(CreateAlignedScaleAnimation(a_duration, a_scaleParmaters));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueMoveToAnimation(float a_duration, Size a_x, Size a_y)
{
	animationQueue.emplace_back(CreateMoveToAnimation(a_duration, a_x, a_y));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueMoveFromAnimation(float a_duration, Size a_x, Size a_y)
{
	animationQueue.emplace_back(CreateMoveToAnimation(a_duration, a_x, a_y));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueFromAlphaAnimation(float a_duration, uint32_t a_alpha)
{
	animationQueue.emplace_back(CreateFromAlphaAnimation(a_duration, a_alpha));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueToAlphaAnimation(float a_duration, uint32_t a_alpha)
{
	animationQueue.emplace_back(CreateToAlphaAnimation(a_duration, a_alpha));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation* ScaleformUI::AnimationHandler::QueueResetAnimation(float a_duration)
{
	animationQueue.emplace_back(CreateResetAnimation(a_duration));
	return &animationQueue.back();
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateScaleFromAnimation(float a_duration, float a_scalePercentage)
{
	Animation animation;
	animation.initialState.SetXScale(element->GetXScale() * a_scalePercentage);
	animation.initialState.SetYScale(element->GetYScale() * a_scalePercentage);
	animation.duration = a_duration;
	return animation;
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateScaleToAnimation(float a_duration, float a_scalePercentage)
{
	Animation animation;
	animation.targetState.SetXScale(element->GetXScale() * a_scalePercentage);
	animation.targetState.SetYScale(element->GetYScale() * a_scalePercentage);
	animation.duration = a_duration;
	return animation;
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateAlignedScaleAnimation(float a_duration, ScaleParameters a_scaleParameters)
{
	Animation animation;
	animation.targetState = AlignedScaleToState(a_scaleParameters);
	animation.duration = a_duration;
	return animation;
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateMoveToAnimation(float a_duration, Size a_x, Size a_y)
{
	Animation animation;
	animation.targetState.SetX(element->SizeToPosition(a_x, true));
	animation.targetState.SetY(element->SizeToPosition(a_y, false));
	animation.duration = a_duration;
	return animation;
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateMoveFromAnimation(float a_duration, Size a_x, Size a_y)
{
	Animation animation;
	animation.initialState.SetX(element->SizeToPosition(a_x, true));
	animation.initialState.SetY(element->SizeToPosition(a_y, false));
	animation.duration = a_duration;
	return animation;
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateFromAlphaAnimation(float a_duration, uint32_t a_alpha)
{
	Animation animation;
	animation.initialState.SetAlpha(a_alpha);
	animation.duration = a_duration;
	return animation;
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateToAlphaAnimation(float a_duration, uint32_t a_alpha)
{
	Animation animation;
	animation.targetState.SetAlpha(a_alpha);
	animation.duration = a_duration;
	return animation;
}

ScaleformUI::AnimationHandler::Animation ScaleformUI::AnimationHandler::CreateResetAnimation(float a_duration)
{
	Animation animation;
	animation.targetState = GetPhysicalState();
	animation.duration = a_duration;
	animation.restoreElementOnAnimationFinish = true;
	return animation;
}

ScaleformUI::AnimationHandler::State ScaleformUI::AnimationHandler::GetPhysicalState()
{
	State newState;
	newState.SetX(element->x);
	newState.SetY(element->y);
	newState.SetXScale(element->xScale);
	newState.SetYScale(element->yScale);
	newState.SetAlpha(element->alpha);
	return newState;
}

ScaleformUI::AnimationHandler::State ScaleformUI::AnimationHandler::AlignedScaleToState(ScaleParameters a_scaleParameters)
{
	State newState;

	float newXScale = element->GetXScale() * a_scaleParameters.scalePercentage;
	float newYScale = element->GetYScale() * a_scaleParameters.scalePercentage;

	newState.SetXScale(newXScale);
	newState.SetYScale(newYScale);

	auto align = CombineAlignments(a_scaleParameters.align1, a_scaleParameters.align2);

	float newWidth = element->GetWidth() / element->GetXScale() * newXScale;
	float newHeight = element->GetHeight() / element->GetYScale() * newYScale;

	if (CompareAlignments(align, Alignment::kLeft))
	{
		newState.SetX(element->x);
	}
	if (CompareAlignments(align, Alignment::kCenterH))
	{
		newState.SetX(element->x + element->GetWidth() / 2 - newWidth / 2);
	}
	if (CompareAlignments(align, Alignment::kRight))
	{
		newState.SetX(element->x + element->GetWidth() - newWidth);
	}
	if (CompareAlignments(align, Alignment::kTop))
	{
		newState.SetY(element->y);
	}
	if (CompareAlignments(align, Alignment::kCenterV))
	{
		newState.SetY(element->y + element->GetHeight() / 2 - newHeight / 2);
	}
	if (CompareAlignments(align, Alignment::kBottom))
	{
		newState.SetY(element->y + element->GetHeight() - newHeight);
	}

	return newState;
}

ScaleformUI::AnimationHandler::State ScaleformUI::AnimationHandler::Animation::InterpolateState()
{
	if (duration == 0.0f) return targetState;

	float t = currentTime / duration;
	if (t > 1.0f) return targetState;

	State newState;

	auto linInterp = [](float xi, float xf, float t) -> float
	{
		return xi + (xf-xi)*t;
	};

	switch (interpolationType)
	{
		case InterpolationType::kLinear:
		{
			if (targetState.hasPosition)
			{
				newState.SetX(linInterp(initialState.GetX(), targetState.GetX(), t));
				newState.SetY(linInterp(initialState.GetY(), targetState.GetY(), t));
			}

			if (targetState.hasScale)
			{
				newState.SetXScale(linInterp(initialState.GetXScale(), targetState.GetXScale(), t));
				newState.SetYScale(linInterp(initialState.GetYScale(), targetState.GetYScale(), t));
			}

			if (targetState.hasAlpha)
			{
				newState.SetAlpha(linInterp(initialState.GetAlpha(), targetState.GetAlpha(), t));
			}

			break;
		}
	}

	return newState;
}

void ScaleformUI::AnimationHandler::SetPostAnimationQueueFinishCallback(UIFunction a_callback, bool a_ensureRun) 
{ 
	if (ensurePostAnimationQueueFinsihCallbackRuns)
	{
		PostAnimationQueueFinish();
	}
	postAnimationQueueFinishCallback = a_callback; 
	ensurePostAnimationQueueFinsihCallbackRuns = a_ensureRun; 
}

void ScaleformUI::AnimationHandler::PostAnimationQueueFinish()
{
	if (postAnimationQueueFinishCallback)
	{
		postAnimationQueueFinishCallback(element);
		ensurePostAnimationQueueFinsihCallbackRuns = false;
		postAnimationQueueFinishCallback = nullptr;
	}
}