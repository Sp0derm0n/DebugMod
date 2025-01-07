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
}