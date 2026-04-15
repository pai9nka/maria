"use strict";loaded_g_0(function(_){var window=this;
_.h("Iyfm0");
var FRd=class extends _.dz{constructor(a){super("userCommentUpdated");this.detail=a}},GRd=function(a){var b=!!a.Ih&&(!a.Ca||!!a.kn);b!==a.za&&(a.za=b,a=a.j,b=new _.cW("submit_button_status_update",{xS:b}),a.dispatchEvent(b))};
_.Xt(class extends _.Yt{constructor(){super(...arguments);this.Ih=null;this.kn=this.ba="";this.aa=this.ea=this.da=this.za=this.Ca=!1;this.xa=this.j=null;this.la=[]}pa(a,b,c){this.Ca=c;this.Ih!==a?(this.Ih=a,this.ba=b):(this.Ih=null,this.ba="");this.la.forEach(d=>{var e=new _.cW("issue_chip_status_update",{Ih:this.Ih});d.dispatchEvent(e)});GRd(this)}Oa(a){if(!!a.length!==!!this.kn.length)if(a.length>0&&!this.Ih){var b=this.j,c=new _.cW("issue_chip_auto_select",{toggle:!0},!1);b.dispatchEvent(c)}else a.length===
0&&this.Ih&&(b=this.j,c=new _.cW("issue_chip_auto_select",{toggle:!1},!1),b.dispatchEvent(c));this.kn=a;GRd(this)}Ea(a){this.da=a}wa(a){this.ea=a}Ma(a){this.j=a}Fa(a){this.la.push(a)}Ia(a){this.xa=a}na(a){this.aa=a}reset(){this.Ih=null;this.kn=this.ba="";this.aa=!1;this.la.forEach(a=>{var b=new _.cW("issue_chip_status_update",{Ih:this.Ih});a.dispatchEvent(b)});this.xa.dispatchEvent(new FRd({kn:this.kn}));GRd(this)}},_.tSa);
_.q();
});
// Google Inc.
