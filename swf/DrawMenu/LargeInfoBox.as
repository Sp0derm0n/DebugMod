class LargeInfoBox extends MovieClip
{
	var infoTextLeft:MovieClip;
	var infoTextRight
	var frame:MovieClip;

	public function  LargeInfoBox()
	{
		super();
		Hide();
	}
	
	public function Hide()
	{
		frame._alpha = 0;
		infoTextLeft._alpha = 0;
		infoTextRight._alpha = 0;
	}
	
	public function Show()
	{
		frame._alpha = 65;
		infoTextLeft._alpha = 100;
		infoTextRight._alpha = 100;
	}
	
	public function SetText(a_text:String)
	{
		infoTextLeft.SetText(a_text);
		infoTextRight.SetText(a_text);
	}
	
	public function Scroll(a_lines:Number)
	{
		//infoTextRight.Scroll(a_lines);
	}
}