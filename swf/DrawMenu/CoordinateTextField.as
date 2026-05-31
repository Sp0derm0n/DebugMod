class CoordinateTextField extends MovieClip
{
	var textField:TextField;

	public function CoordinateTextField()
	{
		super();
	}
	
	public function SetText(a_text:String)
	{
		textField.text = a_text;
	}
	
	public function GetX():Number
	{
		return this._x + this.getBounds().xMin;
	}
	
	public function GetY():Number
	{
		return this._y + this.getBounds().yMin;
	}
}