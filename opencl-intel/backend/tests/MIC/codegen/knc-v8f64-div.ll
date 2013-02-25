; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 


;
; REVIEW: Both tests should generate FMA. Using FAM here is required for 
; correctness
;

target datalayout = "e-p:64:64"

@gb = common global <8 x double> zeroinitializer, align 64
@pgb = common global <8 x double>* null, align 8

define <8 x double> @div1(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:


; KNF: div1:
; KNF: vloadd    
; KNF: vkxnor    
; KNF: vandpq    
; KNF: vandpq    
; KNF: vsubrpi   
; KNF: vrcpresps 
; KNF: vrcprefineps
; KNF: vcvtps2pd 
; KNF: vmsubr23c1pd
; KNF: vkor      
; KNF: vmulpd    
; KNF: vkandn    
; KNF: vmsubr231pd 
; KNF: vmadd231pd 
; KNF: vorpi    



; KNFmpa: vloadd    
; KNFmpa: vkxnor    
; KNFmpa: vandpq    
; KNFmpa: vaddpi    
; KNFmpa: vorpq     
; KNFmpa: vandpq    
; KNFmpa: vandpq    
; KNFmpa: vsubrpi   
; KNFmpa: vcmppd    
; KNFmpa: vcvtpd2ps 
; KNFmpa: vcmppd    
; KNFmpa: vsrlpi    
; KNFmpa: vandpq    
; KNFmpa: vandpq    
; KNFmpa: vrcpresps 
; KNFmpa: vsubpi    
; KNFmpa: vrcprefineps
; KNFmpa: vxorpq    
; KNFmpa: vorpq     
; KNFmpa: vcmppd    
; KNFmpa: vcvtps2pd 
; KNFmpa: vmulpd    
; KNFmpa: vmsubr23c1pd
; KNFmpa: vkor      
; KNFmpa: vmulpd    
; KNFmpa: vkandn    
; KNFmpa: vmadd231pd 
; KNFmpa: vmadd231pd 
; KNFmpa: vmulpd    
; KNFmpa: vmsubr231pd 
; KNFmpa: vmadd231pd 
; KNFmpa: vorpi  
      

  %div = fdiv <8 x double> %a, %b
  ret <8 x double> %div
}

define <8 x double> @div2(<8 x double>* nocapture %a, <8 x double> %b) nounwind readonly ssp {
entry:
    
; KNF: div2:
; KNF:        vandpq    
; KNF:        vorpq     
; KNF:        vandpq    
; KNF:        vrcpresps 
; KNF:        vrcprefineps 
; KNF:        vcvtps2pd       
; KNF:        vmsubr23c1pd
; KNF:        vkor      
; KNF:        vmulpd    
; KNF:        vkandn    
; KNF:        vmsubr213pd
; KNF:        vmadd231pd 
; KNF:        vorpi     



; KNFmpa:        vkxnor    
; KNFmpa:        vandpq    
; KNFmpa:        vaddpi    
; KNFmpa:        vorpq     
; KNFmpa:        vandpq    
; KNFmpa:        vandpq    
; KNFmpa:        vsubrpi   
; KNFmpa:        vcmppd    
; KNFmpa:        vcvtpd2ps 
; KNFmpa:        vcmppd    
; KNFmpa:        vsrlpi    
; KNFmpa:        vandpq    
; KNFmpa:        vandpq    
; KNFmpa:        vrcpresps 
; KNFmpa:        vsubpi       
; KNFmpa:        vrcprefineps 
; KNFmpa:        vxorpq    
; KNFmpa:        vorpq     
; KNFmpa:        vcmppd    
; KNFmpa:        vcvtps2pd       
; KNFmpa:        vmulpd    
; KNFmpa:        vmsubr23c1pd
; KNFmpa:        vkor      
; KNFmpa:        vmulpd    
; KNFmpa:        vkandn    
; KNFmpa:        vmadd231pd
; KNFmpa:        vmadd231pd     
; KNFmpa:        vmulpd    
; KNFmpa:        vmsubr213pd
; KNFmpa:        vmadd231pd 
; KNFmpa:        vorpi     

  %tmp1 = load <8 x double>* %a, align 64
  %div = fdiv <8 x double> %tmp1, %b
  ret <8 x double> %div
}

define <8 x double> @div3(<8 x double> %a, <8 x double>* nocapture %b) nounwind readonly ssp {
; KNF: div3:
; KNF:        vandpq    
; KNF:        vandpq    
; KNF:        vcvtpd2ps 
; KNF:        vrcpresps 
; KNF:        vrcprefineps
; KNF:        vcvtps2pd 
; KNF:        vmsubr23c1pd
; KNF:        vkor      
; KNF:        vmulpd    
; KNF:        vkandn    
; KNF:        vmadd231pd

; KNFmpa:        vandpq    
; KNFmpa:        vaddpi    
; KNFmpa:        vorpq           
; KNFmpa:        vandpq    
; KNFmpa:        vandpq    
; KNFmpa:        vsubrpi   
; KNFmpa:        vcmppd    
; KNFmpa:        vcvtpd2ps 
; KNFmpa:        vcmppd    
; KNFmpa:        vsrlpi    
; KNFmpa:        vandpq    
; KNFmpa:        vandpq    
; KNFmpa:        vrcpresps 
; KNFmpa:        vsubpi    
; KNFmpa:        vrcprefineps
; KNFmpa:        vxorpq    
; KNFmpa:        vorpq     
; KNFmpa:        vcmppd    
; KNFmpa:        vcvtps2pd 
; KNFmpa:        vmulpd    
; KNFmpa:        vmsubr23c1pd
; KNFmpa:        vkor      
; KNFmpa:        vmulpd    
; KNFmpa:        vkandn    
; KNFmpa:        vmadd231pd
; KNFmpa:        vmadd231pd


  %tmp2 = load <8 x double>* %b, align 64
  %div = fdiv <8 x double> %a, %tmp2
  ret <8 x double> %div
}

define <8 x double> @div4(<8 x double> %a) nounwind readonly ssp {
entry:
; KNF: div4:
; KNF:         vandpq    
; KNF:         vorpq    
; KNF:         vandpq 
; KNF:         vcvtpd2ps 
; KNF:         vrcpresps 
; KNF:         vrcprefineps
; KNF:         vmsubr23c1pd
; KNF:         vkor      
; KNF:         vmulpd    
; KNF:         vkandn    
; KNF:         vmadd231pd 

; KNFmpa:         vkxnor    
; KNFmpa:         vandpq    
; KNFmpa:         vaddpi    
; KNFmpa:         vorpq    
; KNFmpa:         vandpq    
; KNFmpa:         vandpq 
; KNFmpa:         vsubrpi   
; KNFmpa:         vcmppd    
; KNFmpa:         vcvtpd2ps 
; KNFmpa:         vcmppd    
; KNFmpa:         vsrlpi    
; KNFmpa:         vandpq    
; KNFmpa:         vandpq    
; KNFmpa:         vrcpresps 
; KNFmpa:         vsubpi    
; KNFmpa:         vrcprefineps
; KNFmpa:         vxorpq    
; KNFmpa:         vorpq     
; KNFmpa:         vcmppd    
; KNFmpa:         vmulpd    
; KNFmpa:         vmsubr23c1pd
; KNFmpa:         vkor      
; KNFmpa:         vmulpd    
; KNFmpa:         vkandn    
; KNFmpa:         vmadd231pd 
; KNFmpa:         vmadd231pd 
; KNFmpa:         vmulpd    
; KNFmpa:         vmsubr231pd
; KNFmpa:         vmadd231pd 



  %tmp1 = load <8 x double>* @gb, align 64
  %div = fdiv <8 x double> %a, %tmp1
  ret <8 x double> %div
}

define <8 x double> @div5(<8 x double> %a) nounwind readonly ssp {
entry:
; KNF: div5:
; KNF:        vandpq    
; KNF:        vandpq    
; KNF:        vrcpresps 
; KNF:        vrcprefineps
; KNF:        vcvtps2pd 
; KNF:        vmsubr23c1pd 
; KNF:        vkor      
; KNF:        vmulpd    
; KNF:        vkandn    
; KNF:        vmsubr231pd

; KNFmpa:        vandpq    
; KNFmpa:        vaddpi    
; KNFmpa:        vorpq     
; KNFmpa:        vandpq    
; KNFmpa:        vandpq    
; KNFmpa:        vsubrpi   
; KNFmpa:        vcmppd    
; KNFmpa:        vcvtpd2ps 
; KNFmpa:        vcmppd
; KNFmpa:        vsrlpi    
; KNFmpa:        vandpq    
; KNFmpa:        vrcpresps 
; KNFmpa:        vsubpi    
; KNFmpa:        vrcprefineps
; KNFmpa:        vxorpq    
; KNFmpa:        vorpq     
; KNFmpa:        vcvtps2pd 
; KNFmpa:        vmulpd    
; KNFmpa:        vmsubr23c1pd 
; KNFmpa:        vkor      
; KNFmpa:        vmulpd    
; KNFmpa:        vkandn    
; KNFmpa:        vmadd231pd 
; KNFmpa:        vmadd231pd 
; KNFmpa:        vmulpd    
; KNFmpa:        vmsubr231pd

  %tmp1 = load <8 x double>** @pgb, align 8
  %tmp2 = load <8 x double>* %tmp1, align 64
  %div = fdiv <8 x double> %a, %tmp2
  ret <8 x double> %div
}
