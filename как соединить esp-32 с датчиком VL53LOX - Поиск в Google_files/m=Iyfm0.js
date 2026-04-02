"use strict";loaded_g_0(function(_){var window=this;
_.h("Iyfm0");
var DNd=class extends _.Ty{constructor(a){super("userCommentUpdated");this.detail=a}},ENd=function(a){var b=!!a.Hh&&(!a.Ea||!!a.gn);b!==a.Ca&&(a.Ca=b,a=a.j,b=new _.FV("submit_button_status_update",{gS:b}),a.dispatchEvent(b))};
_.Ut(class extends _.Vt{constructor(){super(...arguments);this.Hh=null;this.gn=this.ba="";this.aa=this.la=this.da=this.Ca=this.Ea=!1;this.ya=this.j=null;this.na=[]}wa(a,b,c){this.Ea=c;this.Hh!==a?(this.Hh=a,this.ba=b):(this.Hh=null,this.ba="");this.na.forEach(d=>{var e=new _.FV("issue_chip_status_update",{Hh:this.Hh});d.dispatchEvent(e)});ENd(this)}Ua(a){if(!!a.length!==!!this.gn.length)if(a.length>0&&!this.Hh){var b=this.j,c=new _.FV("issue_chip_auto_select",{toggle:!0},!1);b.dispatchEvent(c)}else a.length===
0&&this.Hh&&(b=this.j,c=new _.FV("issue_chip_auto_select",{toggle:!1},!1),b.dispatchEvent(c));this.gn=a;ENd(this)}Fa(a){this.da=a}xa(a){this.la=a}Oa(a){this.j=a}Ia(a){this.na.push(a)}Ma(a){this.ya=a}pa(a){this.aa=a}reset(){this.Hh=null;this.gn=this.ba="";this.aa=!1;this.na.forEach(a=>{var b=new _.FV("issue_chip_status_update",{Hh:this.Hh});a.dispatchEvent(b)});this.ya.dispatchEvent(new DNd({gn:this.gn}));ENd(this)}},_.mSa);
_.q();
});
// Google Inc.
