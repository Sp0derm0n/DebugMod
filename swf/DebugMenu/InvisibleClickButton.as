class InvisibleClickButton extends ClickButton
{
    function InvisibleClickButton()
    {
		super();
		this._visible = false;
    }
	
	function HOVER_OFF()
    {
		this._visible = true;
		this._xscale = 100;
		this._yscale = 100;
    }
	
	function HOVER_ON()
    {
		this._visible = true;
		this._xscale = 100;
		this._yscale = 100;
    }
	
	function HIT()
	{
		this._xscale = 80;
		this._yscale = 80;
	}
	
	function ON()
	{
		this._visible = true;
		this._xscale = 100;
		this._yscale = 100;
	}
   
    function OFF()
    {
		this._visible = false;
		this._xscale = 100;
		this._yscale = 100;
    }
}
