class TextBox extends MovieClip
{
	
	var textField:TextField;
	
    public function TextBox()
    {
		super();
    }
	
	public function SetText(a_text:String)
	{
		textField.text = a_text;
	}
	
	public function SetFontSize(a_size:Number)
	{
		var format:TextFormat;
		format = textField.getTextFormat();
		format.size = a_size;
		textField.setTextFormat(format);
	}
	
	public function CreateClone(a_name:String)
	{
		this.duplicateMovieClip(a_name, 0);
	}
	
	public function SetPosition(a_x:Number, a_y:Number)
	{
		this._x = a_x;
		this._y = a_y;
	}
	
	public function SetWidth(a_width:Number)
	{
		textField._width = a_width;
	}
	
	public function SetTextAlignment(a_alignment:String) // "left", "center", "right"
	{
		var format:TextFormat;
		format = textField.getTextFormat();
		format.align = a_alignment;
		textField.setTextFormat(format);
	}
	
	public function SetTextColor(a_color:Number) // RRGGBB
	{
		var format:TextFormat;
		format = textField.getTextFormat();
		format.color = a_color;
		textField.setTextFormat(format);
	}
	
	public function Hide()
	{
		this._visible = false;
	}
}
