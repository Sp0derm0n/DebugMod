class InfoTextField extends MovieClip
{
	var infoTextField:TextField;

	public function InfoTextField()
	{
		super();
	}
	
	public function SetText(a_text:String)
	{
		infoTextField.text = a_text;
	}
	
	public function FitsText():Boolean
	{
		//return infoTextField.textHeight < infoTextField._height+300; // or scroll = maxscroll
		return infoTextField.scroll == infoTextField.maxscroll;
	}
	
	public function IsScrolled():Boolean
	{
		return infoTextField.scroll != 1;
	}
	
	public function SetScroll(a_scroll:Number)
	{
		infoTextField.scroll = a_scroll;
	}
	
	public function GetMaxScroll():Number
	{
		return infoTextField.maxscroll;
	}
	
}