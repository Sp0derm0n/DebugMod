class CoordinatesBox extends MovieClip
{
	var frame:MovieClip;
	var xTextField:MovieClip;
	var yTextField:MovieClip;
	var zTextField:MovieClip;
	
	public function CoordinatesBox()
	{
		super();
		frame._alpha = 65;
	}
	
	public function SetCoordinates(a_x:String, a_y:String, a_z:String)
	{
		xTextField.SetText(a_x);
		yTextField.SetText(a_y);
		zTextField.SetText(a_z);
	}
	
	public function Hide()
	{
		frame._visible = false;
		xTextField._visible = false;
		yTextField._visible = false;
		zTextField._visible = false;
	}
	
	public function Show()
	{
		frame._visible = true;
		xTextField._visible = true;
		yTextField._visible = true;
		zTextField._visible = true;
	}
	
}