class InfoBox extends MovieClip
{
	var infoText:MovieClip;
	var frame:MovieClip;
	var scrollUp:MovieClip;
	var scrollDown:MovieClip;

	public function InfoBox()
	{
		super();
		frame._alpha = 65;
	}
	
	public function Hide()
	{
		frame._visible = false;
		infoText._visible = false;
		scrollUp._visible = false;
		scrollDown._visible = false;
	}
	
	public function Show()
	{
		frame._visible = true;
		infoText._visible = true;
	}
	
	public function SetText(a_text:String)
	{
		infoText.SetText(a_text);
		ShowArrows();
	}
	
	public function ScrollUp(a_scroll:Number)
	{
		infoText.SetScroll(a_scroll);
		if (scrollUp._visible == true)
		{
			infoText.SetScroll(a_scroll);
			ShowArrows();
		}
	}
	
	public function ScrollDown(a_scroll:Number)
	{
		if (scrollDown._visible == true)
		{
			infoText.SetScroll(a_scroll);
			ShowArrows();
		}
	}
	
	public function GetMaxScroll():Number
	{
		return infoText.GetMaxScroll();
	}
	
	public function ShowArrows()
	{
		if (infoText.FitsText())
		{
			scrollDown._visible = false;
		}
		else
		{
			scrollDown._visible = true;
		}
		
		if (infoText.IsScrolled())
		{
			scrollUp._visible = true;
		}
		else 
		{
			scrollUp._visible = false;
		}
	}
}